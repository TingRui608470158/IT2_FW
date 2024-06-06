/******************************************************************************
* (c) COPYRIGHT 2017 RAONTECH, Inc. ALL RIGHTS RESERVED.                      *
*                                                                             *
* This software is the property of RAONTECH and is furnished under license    *
* by RAONTECH.                                                                *
* This software may be used only in accordance with the terms of said license.*
* This copyright noitce may not be removed, modified or obliterated without   *
* the prior written permission of RAONTECH, Inc.                              *
*                                                                             *
* This software may not be copied, transmitted, provided to or otherwise      *
* made available to any other person, company, corporation or other entity    *
* except as specified in the terms of said license.                           *
*                                                                             *
* No right, title, ownership or other interest in the software is hereby      *
* granted or transferred.                                                     *
*                                                                             *
* The information contained herein is subject to change without notice        *
* and should not be construed as a commitment by RAONTECH, Inc.               *
******************************************************************************/

/*
 Downloader of LUT 
*/

#include "rti_vc_dload.h"
#include "rti_vc_rdc.h"

/**
* External variables.
*/
extern VC_RDC_CB_T vc_rdc_cb;

#define MAX_DLOAD_VERIFY_BUF_SIZE		512

#if defined(CFG_MDC_RDC200)
#include "rdc200_flash_partitions.h"

#define MAX_NUM_DLOAD_ITEM_BUF_POOL		512

#elif defined(CFG_MDC_RDC200A)
#include "rdc200a_flash_partitions.h"

#define MAX_NUM_DLOAD_ITEM_BUF_POOL		512

#else
	#error "Code not present"
#endif

typedef struct
{
	E_VC_DLOAD_STATE_T state;

	volatile BOOL_T stop_flag;

	BOOL_T verify_flag;

	VC_MUTEX_DECLARE(item_lock)

	/* Item buffer pool queue. */
	struct VLIST_T free_q;
	UINT_T free_q_cnt;

	VC_DLOAD_ITEM_LIST_T item_list;

	/* Called by downloader when the erase operation processing. */
	VC_DLOAD_ERASE_CALLBACK_T erase_callback;

	/* Called by downloader when the program operation processing. */
	VC_DLOAD_PROGRAM_CALLBACK_T program_callback;

	/* Called by downloader when the verifying of programmed data operation processing. */
	VC_DLOAD_VERIFY_CALLBACK_T verify_callback;

	/* Called by downloader when the total item processing. */
	VC_DLOAD_PROC_CALLBACK_T proc_callback;
} VC_DLOAD_CB_T;

static VC_DLOAD_CB_T vc_dload_cb;

static BOOL_T dload_init_flag = FALSE;

static U8_T dload_verify_buf[MAX_DLOAD_VERIFY_BUF_SIZE];

static VC_DLOAD_ITEM_INFO_T dload_item_buf_pool[MAX_NUM_DLOAD_ITEM_BUF_POOL];

static void dload_free_item_buf(VC_DLOAD_ITEM_INFO_T *item_buf)
{
	VLIST_INSERT_TAIL(&vc_dload_cb.free_q, &item_buf->link);
	vc_dload_cb.free_q_cnt++;
}

// This function allocates a download item buffer from pool.
static VC_DLOAD_ITEM_INFO_T *dload_alloc_item_buf(void)
{	
	struct VLIST_T *list;
	VC_DLOAD_ITEM_INFO_T *item = NULL;

	list = VLIST_FIRST(&vc_dload_cb.free_q);
	if (list) {
		VLIST_REMOVE(list);
		vc_dload_cb.free_q_cnt--;

		item = VLIST_ENTRY(list, VC_DLOAD_ITEM_INFO_T, link);
	}

	return item;
}

static void dload_init_item_pool(void)
{
	UINT_T i;
	
	VLIST_HEAD_INIT(&vc_dload_cb.free_q);
	vc_dload_cb.free_q_cnt = 0;

	for (i = 0; i < MAX_NUM_DLOAD_ITEM_BUF_POOL; i++) {
		VLIST_INSERT_TAIL(&vc_dload_cb.free_q, &dload_item_buf_pool[i].link);
		vc_dload_cb.free_q_cnt++;
	}
}

static E_VC_DLOAD_STATUS_T dload_verify(VC_RDC_DEV_INFO_T *dev,
										VC_DLOAD_ITEM_INFO_T *item)
{
	int i, ret;
	U16_T verified_bytes = 0;
	U32_T addr = item->pgm_addr;
	U16_T size = item->pgm_data_size, offset;
	U16_T unit = item->pgm_unit_size;
	U16_T read_len, inprogress_bytes = 0;

	vc_dload_cb.state = VC_DLOAD_STATE_VERIFY;

	if (vc_dload_cb.verify_callback)
		vc_dload_cb.verify_callback(VC_DLOAD_STATUS_STARTING, item->lut_class,
									addr, 0, size);

	for (offset = 0; offset < size; ) {		
		if (vc_dload_cb.stop_flag == TRUE) {
			if (vc_dload_cb.verify_callback)
				vc_dload_cb.verify_callback(VC_DLOAD_STATUS_FORCE_STOP,
											item->lut_class,
											addr, verified_bytes, size);

			return VC_DLOAD_STATUS_FORCE_STOP;
		}

		read_len = MIN(size - offset, unit);
		read_len = MIN(read_len, MAX_DLOAD_VERIFY_BUF_SIZE);

		ret = dev->flash_read(addr, dload_verify_buf, read_len);
		if (ret < 0) {
			EMSG("[VC_DLOAD] Can't read the data of FLASH to verify.\n");
			if (vc_dload_cb.verify_callback)
				vc_dload_cb.verify_callback(VC_DLOAD_STATUS_DEV_FAIL,
											item->lut_class,
											addr, verified_bytes, size);
			return VC_DLOAD_STATUS_DEV_FAIL;
		}

		for (i = 0; i < read_len; i++) {
			if (dload_verify_buf[i] != item->pgm_data[i + offset]) {
				EMSG("[VC_DLOAD] FLASH verify error!\n");
				if (vc_dload_cb.verify_callback)
					vc_dload_cb.verify_callback(VC_DLOAD_STATUS_VERIFY_ERR,
												item->lut_class,
												addr, verified_bytes, size);
				return VC_DLOAD_STATUS_DEV_FAIL;
			}
			verified_bytes++;
		}

		if (verified_bytes != size) {
			inprogress_bytes += read_len;

			if (inprogress_bytes >= unit) {
				if (vc_dload_cb.verify_callback)
					vc_dload_cb.verify_callback(VC_DLOAD_STATUS_INPROGRESS,
												item->lut_class,
												addr, verified_bytes, size);
		
				inprogress_bytes = 0;
			}
		}
		else {
			if (vc_dload_cb.verify_callback)
				vc_dload_cb.verify_callback(VC_DLOAD_STATUS_COMPLTED,
											item->lut_class,
											addr, verified_bytes, size);
		}

		addr += read_len;
		offset += read_len;
	}

	return VC_DLOAD_STATUS_COMPLTED;
}

static E_VC_DLOAD_STATUS_T dload_program(VC_RDC_DEV_INFO_T *dev,
										 VC_DLOAD_ITEM_INFO_T *item)
{
	int ret;
	U16_T programmed_bytes = 0;
	U32_T addr = item->pgm_addr;
	U16_T size = item->pgm_data_size;
	U16_T unit = item->pgm_unit_size; /* To use progress */
	U16_T pgm_len;
	E_VC_DLOAD_STATUS_T status;
	U8_T *pgm_buf_ptr = item->pgm_data;

	vc_dload_cb.state = VC_DLOAD_STATE_PROGRAM;

	if (vc_dload_cb.program_callback)
		vc_dload_cb.program_callback(VC_DLOAD_STATUS_STARTING,
									item->lut_class, addr,
									0, item->pgm_data_size);

	while (size > 0) {
		if (vc_dload_cb.stop_flag == TRUE) {
			if (vc_dload_cb.program_callback)
				vc_dload_cb.program_callback(VC_DLOAD_STATUS_FORCE_STOP,
											item->lut_class,
											addr, programmed_bytes,
											item->pgm_data_size);

			return VC_DLOAD_STATUS_FORCE_STOP;
		}

		pgm_len = MIN(size, unit);

		//DMSG("addr: 0x%X\n", addr);

		ret = dev->flash_write(addr, pgm_buf_ptr, pgm_len);
		if (ret) {
			EMSG("[VC_DLOAD] Can't FLASH program! Retry again.\n");
			if (vc_dload_cb.program_callback)
				vc_dload_cb.program_callback(VC_DLOAD_STATUS_DEV_FAIL,
											item->lut_class,
											addr, programmed_bytes,
											item->pgm_data_size);
			return VC_DLOAD_STATUS_DEV_FAIL;
		}

		programmed_bytes += pgm_len;
		if (programmed_bytes != item->pgm_data_size)
			status = VC_DLOAD_STATUS_INPROGRESS;
		else
			status = VC_DLOAD_STATUS_COMPLTED;

		if (vc_dload_cb.program_callback)
			vc_dload_cb.program_callback(status, item->lut_class,
										addr, programmed_bytes,
										item->pgm_data_size);

		pgm_buf_ptr += pgm_len;
		addr += pgm_len;
		size -= pgm_len;
	}

	return VC_DLOAD_STATUS_COMPLTED;
}

static E_VC_DLOAD_STATUS_T dload_erase_sector(VC_RDC_DEV_INFO_T *dev,
											  VC_DLOAD_ITEM_INFO_T *item)
{
	int i, ret;
	U32_T sector_addr;
	U16_T num_sector, sector_size, first_blank_len;
	E_VC_DLOAD_STATUS_T status;
	U16_T erased_sector_cnt = 0;

	vc_dload_cb.state = VC_DLOAD_STATE_ERASE;

	sector_size = 1 << dev->sector_size_shift;	
	sector_addr = ALIGN_FLASH_SECTOR(item->pgm_addr, sector_size);

	first_blank_len = item->pgm_addr - sector_addr;

	/* Sector align DIV_ROUND_UP() */
	num_sector
		= (first_blank_len + item->pgm_data_size + (sector_size-1))
				>> dev->sector_size_shift;

	//DMSG("sector_size(0x%X), num_sector(%d)\n", sector_size, num_sector);

	if (vc_dload_cb.erase_callback)
		vc_dload_cb.erase_callback(VC_DLOAD_STATUS_STARTING, item->lut_class,
									sector_addr, 0, num_sector);

	for (i = 0; i < num_sector; i++) {		
		if (vc_dload_cb.stop_flag == TRUE) {
			if (vc_dload_cb.erase_callback)
				vc_dload_cb.erase_callback(VC_DLOAD_STATUS_FORCE_STOP,
									item->lut_class,
									sector_addr, erased_sector_cnt, num_sector);

			return VC_DLOAD_STATUS_FORCE_STOP;
		}

		/* Erase a sector. */
		ret = dev->flash_erase_sector(sector_addr);
		if (ret) {
			EMSG("[VC_DLOAD] Can't do FLASH SECTOR ERASE! Retry again.\n");
			if (vc_dload_cb.erase_callback)
				vc_dload_cb.erase_callback(VC_DLOAD_STATUS_DEV_FAIL,
										item->lut_class, sector_addr,
										erased_sector_cnt, num_sector);
			return VC_DLOAD_STATUS_DEV_FAIL;
		}

		erased_sector_cnt++;
		if (erased_sector_cnt != num_sector)
			status = VC_DLOAD_STATUS_INPROGRESS;
		else
			status = VC_DLOAD_STATUS_COMPLTED;

		if (vc_dload_cb.erase_callback)
			vc_dload_cb.erase_callback(status, item->lut_class, sector_addr,
										erased_sector_cnt, num_sector);

		sector_addr += sector_size;	
	}

	return VC_DLOAD_STATUS_COMPLTED;
}

static void dload_remove_all_item_list(E_VC_DLOAD_STATUS_T status,
									int processing_cnt, int num_total_item)
{
	struct VLIST_T *pos, *tmp;
	VC_DLOAD_ITEM_INFO_T *item;

	/*
	 NOTE: Mutex should locked before this function was called.
	*/

	VLIST_FOREACH_REMOVAL(&vc_dload_cb.item_list.head, pos, tmp) {
		item = VLIST_ENTRY(pos, VC_DLOAD_ITEM_INFO_T, link);

		VLIST_REMOVE(&item->link);
		vc_dload_cb.item_list.cnt--;

		VC_MUTEX_FREE(&vc_dload_cb.item_lock);

		if (vc_dload_cb.proc_callback)
			vc_dload_cb.proc_callback(status, item->lut_class, item->pgm_data,
									processing_cnt, num_total_item);
			
		VC_MUTEX_LOCK(&vc_dload_cb.item_lock);

		dload_free_item_buf(item);
	}
}

#if 0
static void delete_dload_item(VC_DLOAD_ITEM_INFO_T *item)
{
	VLIST_REMOVE(&item->link);
	vc_dload_cb.item_list.cnt--;

	item->proc_callback(item);
	dload_free_item_buf(item);
}
#endif

static void dlaod_insert_item(VC_DLOAD_ITEM_INFO_T *item)
{
	VLIST_INSERT_TAIL(&vc_dload_cb.item_list.head, &item->link);
	vc_dload_cb.item_list.cnt++;
}

void rtiVC_StopDownload(void)
{
	vc_dload_cb.stop_flag = TRUE;
}

void rtiVC_StartDownload(void)
{
	int ret = 0;
	struct VLIST_T *pos, *tmp;
	VC_DLOAD_ITEM_INFO_T *item = NULL;
	VC_RDC_DEV_INFO_T *dev;
	int num_total_item = 0, processing_cnt = 0;
	E_VC_DLOAD_STATUS_T status = VC_DLOAD_STATUS_STARTING;

	vc_dload_cb.stop_flag = FALSE;

	dev = &vc_rdc_cb.dev;

	VC_MUTEX_LOCK(&vc_dload_cb.item_lock);

	num_total_item = vc_dload_cb.item_list.cnt;

	if (dev->flash_prepare) {
		ret = dev->flash_prepare();
		if (ret) {
			status = VC_DLOAD_STATUS_DEV_FAIL;
			goto exit;
		}
	}

	if (vc_dload_cb.proc_callback) {
		tmp = VLIST_FIRST(&vc_dload_cb.item_list.head);
		if (tmp != NULL) {
			item = VLIST_ENTRY(tmp, VC_DLOAD_ITEM_INFO_T, link);
	
			vc_dload_cb.proc_callback(VC_DLOAD_STATUS_STARTING, item->lut_class,
									item->pgm_data,
									0, num_total_item);
		}
	}

	VLIST_FOREACH_REMOVAL(&vc_dload_cb.item_list.head, pos, tmp) {
		item = VLIST_ENTRY(pos, VC_DLOAD_ITEM_INFO_T, link);

		// Remove from item list.
		VLIST_REMOVE(&item->link);
		vc_dload_cb.item_list.cnt--;

		VC_MUTEX_FREE(&vc_dload_cb.item_lock);

		processing_cnt++;

		status = dload_erase_sector(dev, item);
		if (status != VC_DLOAD_STATUS_COMPLTED)
			goto exit;

		status = dload_program(dev, item);
		if (status != VC_DLOAD_STATUS_COMPLTED)
			goto exit;

		if (vc_dload_cb.verify_callback) {
			status = dload_verify(dev, item);
			if (status != VC_DLOAD_STATUS_COMPLTED)
				goto exit;
		}

		if (vc_dload_cb.item_list.cnt != 0)
			status = VC_DLOAD_STATUS_INPROGRESS;
		else
			status = VC_DLOAD_STATUS_COMPLTED;

		if (vc_dload_cb.proc_callback)
			vc_dload_cb.proc_callback(status, item->lut_class,
										item->pgm_data,
										processing_cnt, num_total_item);

		VC_MUTEX_LOCK(&vc_dload_cb.item_lock);
		dload_free_item_buf(item);
		item = NULL;
	}

exit:
	if (dev->flash_finish)
		dev->flash_finish();

	if (item)
		dload_free_item_buf(item);

	if (status != VC_DLOAD_STATUS_COMPLTED)
		dload_remove_all_item_list(status, processing_cnt, num_total_item);

	VC_MUTEX_FREE(&vc_dload_cb.item_lock);
}

void rtiVC_RemoveAllSetupItem(void)
{
	VC_MUTEX_LOCK(&vc_dload_cb.item_lock);

	vc_dload_cb.stop_flag = FALSE;
	
	dload_remove_all_item_list(VC_DLOAD_STATUS_REMOVE_SETUP_ITEM, 0, 0);
	
	vc_dload_cb.state = VC_DLOAD_STATE_INIT;

	VC_MUTEX_FREE(&vc_dload_cb.item_lock);
}

int rtiVC_SetupDownloadItem(U32_T pgm_addr, U8_T *pgm_data,
							U16_T pgm_data_size,
							U16_T pgm_unit_size, E_VC_LUT_CLASS_T lut_class)
{
	int ret = 0;
	VC_DLOAD_ITEM_INFO_T *item = NULL;

	VC_MUTEX_LOCK(&vc_dload_cb.item_lock);

	item = (VC_DLOAD_ITEM_INFO_T *)dload_alloc_item_buf();
	if (item != NULL) {
		item->pgm_addr = pgm_addr;
		item->pgm_data = pgm_data;
		item->pgm_data_size = pgm_data_size;
		item->pgm_unit_size = pgm_unit_size;
		item->lut_class = lut_class;

		dlaod_insert_item(item);
	}
	else {
		EMSG("Fail to allocate a buffer of download item info!\n");
		ret = 1;
	}

	VC_MUTEX_FREE(&vc_dload_cb.item_lock);

	return ret;
}

void rtiVC_RegisterDownloadCallback(VC_DLOAD_ERASE_CALLBACK_T erase_callback,
									VC_DLOAD_PROGRAM_CALLBACK_T program_callback,
									VC_DLOAD_VERIFY_CALLBACK_T verify_callback,
									VC_DLOAD_PROC_CALLBACK_T proc_callback)
{
	vc_dload_cb.erase_callback = erase_callback;
	vc_dload_cb.program_callback = program_callback;
	vc_dload_cb.verify_callback = verify_callback;
	vc_dload_cb.proc_callback = proc_callback;
}

void rtiVC_DeinitDownload(void)
{
	if (dload_init_flag) {
		VC_MUTEX_LOCK(&vc_dload_cb.item_lock);

		vc_dload_cb.stop_flag = FALSE;

		dload_remove_all_item_list(VC_DLOAD_STATUS_SHUTDOWN, 0, 0);

		VC_MUTEX_FREE(&vc_dload_cb.item_lock);
		
		VC_MUTEX_DEINIT(&vc_dload_cb.item_lock);

		vc_dload_cb.state = VC_DLOAD_STATE_INIT;

		dload_init_flag = FALSE;
	}
}

void rtiVC_InitDownload(void)
{
	memset(&vc_dload_cb, 0, sizeof(vc_dload_cb));

	dload_init_item_pool();

	VC_MUTEX_INIT(&vc_dload_cb.item_lock);
	
	VLIST_HEAD_INIT(&vc_dload_cb.item_list.head);
	vc_dload_cb.item_list.cnt = 0;

	dload_init_flag = TRUE;

	vc_dload_cb.state = VC_DLOAD_STATE_INIT;
}

