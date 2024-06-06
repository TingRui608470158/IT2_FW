
#include <stdio.h>

#include "rdc_lut_patch.h"
#include "rti_vc_common.h"
#if defined(CFG_MDC_RDC200)
	#include "rdc200_flash_partitions.h"

#elif defined(CFG_MDC_RDC200A)
	#include "rdc200a_flash_partitions.h"
#else
	#error "Code not present"
#endif

/**
 LED current for each panel and pixel shift for RDC.
 */
#if defined(CFG_PANEL_RDP551F)
	static const U16_T rdp551f_pxshift_patch_reg_list[]
	 = {0x001B, 0x001C};

	static const U16_T rdp551f_ledcurr_patch_reg_list[] // LED current
	= {0x10E, 0x10F, 0x110};

	static const U16_T *rdc200_pxshift_patch_reg_list = NULL;

#else
	#error "Code not present"
#endif

/**
 LED duty for each RDC.
 */
#if defined(CFG_MDC_RDC200)
	static const U16_T rdc200_ledduty_patch_reg_list[] // LED duty
		= {0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50};

#elif defined(CFG_MDC_RDC200A)
	static const U16_T rdc200_ledduty_patch_reg_list[] = { // LED duty
		0x0853, 0x0854, 0x0855, 0x0856, 0x085B, 0x085C, 0x085D, 0x085E,
		0x0863, 0x0864, 0x0865, 0x0866,
		0x0953, 0x0954, 0x0955, 0x0956, 0x095B, 0x095C, 0x095D, 0x095E,
		0x0963, 0x0964, 0x0965, 0x0966,		
	};
#endif

#define MAX_NUM_APPEND_LUT_NODE\
	(FLASH_PART_SIZE_RDC_PATCH / sizeof(RD_LUT_NODE_T))

static RD_LUT_NODE_T append_list[MAX_NUM_APPEND_LUT_NODE];

static U16_T manual_rdc_patch_reg_list[MAX_NUM_APPEND_LUT_NODE];
static int num_manual_rdc_patch_reg_list = 0;

static U16_T manual_rdp_patch_reg_list[MAX_NUM_VC_PANEL_PORT][MAX_NUM_APPEND_LUT_NODE];
static int num_manual_rdp_patch_reg_list[MAX_NUM_VC_PANEL_PORT] = {0, 0};;

/**
 Update the old value by the patch value.
 first_node: Pointer to the first RDC node in patch buffer.
 num_reg_list: The number of register to be update.
*/
static int update_rdc_flash_patch_data(RD_LUT_NODE_T *first_node, int num_flash_node,
									const U16_T *reg_list, int num_reg_list)
{
	int i, k;
	BOOL_T updated = FALSE;	
	int num_append = 0;
	RD_LUT_NODE_T *node_ptr;

	if (reg_list == NULL) {
		return num_flash_node;
	}

	/* 1. Replace the old value by the patch value for the exist register. */
	for (i = 0; i < num_reg_list; i++) {
		updated = FALSE;
		node_ptr = first_node;

		for (k = 0; k < num_flash_node; k++) {
			if (reg_list[i] == SWAP16(node_ptr->reg)) {
				node_ptr->val = RDC_REG_GET(reg_list[i]); /* Replace */
				updated = TRUE;
				break;
			}

			node_ptr++;
		}

		if (updated == FALSE) { /* Save the append node */
			append_list[num_append].reg = reg_list[i];
			append_list[num_append].val = RDC_REG_GET(reg_list[i]);
			num_append++;
		}
	}

	/* 2. Append the new node at the end of exist node. */
	node_ptr = first_node + num_flash_node;
	
	for (i = 0; i < num_append; i++) {
		node_ptr->reg = SWAP16(append_list[i].reg);
		node_ptr->val = RDC_REG_GET(reg_list[i]);
	
		node_ptr++;
	}
	
	return (num_flash_node + num_append);
}

static int build_rdc_flash_patch_data(U8_T *rdc_buf, E_VC_RDC_PATCH_T ptype)
{
	int ret = 0;
	RD_LUT_HEADER_T phdr;
	RD_LUT_NODE_T *rdc_node;
	RD_LUT_RECORD_T *rdc_record;
	U32_T lut_node_size; /* The size of LUT node in bytes. */
	int num_flash_node = 0, lut_record_size;

	// Read LUT header
	ret = rtiVC_ReadFLASH(FLASH_PART_OFFSET_RDC_PATCH_BASE,
							(U8_T *)&phdr, sizeof(RD_LUT_HEADER_T));
	if (ret != 0) {
		EMSG("Fail to read the RDC patch header in FLASH\n");
		return -1;
	}

	if (phdr.magic_code == SWAP32(FLASH_RDC_PATCH_MAGIC_CODE)) {
		num_flash_node = SWAP16(phdr.num_node);
		lut_node_size = num_flash_node * sizeof(RD_LUT_NODE_T);

		// Read LUT nodes
		rtiVC_ReadFLASH(FLASH_PART_OFFSET_RDC_PATCH_BASE + sizeof(RD_LUT_HEADER_T),
						rdc_buf + sizeof(RD_LUT_HEADER_T),
						lut_node_size);
	}
	else {
		if(phdr.magic_code != 0xFFFFFFFF) {
			EMSG("Magic code warning!.\n");
			rtiVC_EraseSectorFLASH(FLASH_PART_OFFSET_RDC_PATCH_BASE);
		}

		num_flash_node = 0;
	}

	rdc_record = (RD_LUT_RECORD_T *)rdc_buf;
	rdc_node = &rdc_record->node[0]; // Get the first position of node in FLASH.

#if defined(CFG_PANEL_RDP501H)
	if (ptype == 0)
		num_flash_node = update_rdc_flash_patch_data(rdc_node, num_flash_node,
								rdp501h_ledcurr_patch_reg_list,
								ARRAY_SIZE(rdp501h_ledcurr_patch_reg_list));

#endif

	if (ptype & VC_RDC_PIXSEL_SHIFT) {
		num_flash_node = update_rdc_flash_patch_data(rdc_node, num_flash_node,
								rdc200_pxshift_patch_reg_list,
								ARRAY_SIZE(rdc200_pxshift_patch_reg_list));
	}

	if (ptype & VC_RDC_LED_DUTY) {
		num_flash_node = update_rdc_flash_patch_data(rdc_node, num_flash_node,
								rdc200_ledduty_patch_reg_list,
								ARRAY_SIZE(rdc200_ledduty_patch_reg_list));
	}

	if (ptype & VC_RDC_MANUAL_PATCH) {
		num_flash_node = update_rdc_flash_patch_data(rdc_node, num_flash_node,
								manual_rdc_patch_reg_list,
								num_manual_rdc_patch_reg_list);
	}
	
	lut_node_size = num_flash_node * sizeof(RD_LUT_NODE_T);
	
	/* Fill the RDC LUT header. */
	rdc_record->hdr.magic_code = SWAP32(FLASH_RDC_PATCH_MAGIC_CODE);
	rdc_record->hdr.num_node = SWAP16(num_flash_node); // Store as big endian.

	lut_record_size = sizeof(RD_LUT_HEADER_T) + lut_node_size;

	return lut_record_size;
}

static int update_panel_flash_patch_data(int panel_port,
								RD_LUT_NODE_T *first_node, int num_flash_node,
								const U16_T *reg_list, int num_reg_list)
{
	int i, k;
	BOOL_T updated = FALSE;
	int num_append = 0; // The number of new node to be appended.
	RD_LUT_NODE_T *node_ptr;
	int total_node;
#if defined(CFG_PANEL_RDP550F)
	BOOL_T first_0x8A_found = FALSE;
	U8_T shift_h, reg_0x8A_top, reg_0x8A_bottom;
#endif

	if (reg_list == NULL) {
		return num_flash_node;
	}

	/* 1. Replace the old value by the patch value for the specified register was exist. */
	node_ptr = first_node;

	for (i = 0; i < num_reg_list; i++) {
		updated = FALSE;
		node_ptr = first_node;

		for (k = 0; k < num_flash_node; k++) {
			if (reg_list[i] == SWAP16(node_ptr->reg)) {
				node_ptr->val = RDP_REG_GET(panel_port, reg_list[i]); /* Replace */
				updated = TRUE;
				break;
			}
			node_ptr++;
		}

		if (updated == FALSE) { /* Save the append node */
			append_list[num_append].val = RDP_REG_GET(panel_port, reg_list[i]);
			append_list[num_append].reg = reg_list[i];
			num_append++;
		}
	}

	/* 2. Append the new node at the end of exist node. */
	node_ptr = first_node + num_flash_node;
	
	for (i = 0; i < num_append; i++) {
		node_ptr->reg = SWAP16(append_list[i].reg);		
		node_ptr->val = RDP_REG_GET(panel_port, reg_list[i]);	
		node_ptr++;
	}

	total_node = num_flash_node + num_append;

	/* Update the bottom of register 0x8A. */
#if defined(CFG_PANEL_RDP550F)
	node_ptr = first_node;

	shift_h = RDP_REG_GET(panel_port, 0x1D) & 0x1F; // SHIFT_H<4:0>	
	reg_0x8A_bottom = RDP_REG_GET(panel_port, 0x8A);
	reg_0x8A_top = reg_0x8A_bottom + (shift_h % 2);

	for (i = 0; i < total_node; i++) {
		if (SWAP16(node_ptr->reg) == 0x8A) {
			if (first_0x8A_found == FALSE) {
				node_ptr->val = reg_0x8A_top;
				first_0x8A_found = TRUE;
			}
			else {
				node_ptr->val = reg_0x8A_bottom;
				break;
			}
		}
		node_ptr++;
	}
#endif

	return total_node;
}

static int build_panel_flash_patch_data(int panel_port, U8_T *panel_buf,
										E_VC_PANEL_PATCH_T ptype)
{
	U32_T flash_addr, magic_code;
	int ret = 0;
	RD_LUT_HEADER_T phdr;
	RD_LUT_NODE_T *panel_node;
	RD_LUT_RECORD_T *panel_record;
	int lut_node_size; /* The size of LUT node in bytes. */
	int num_flash_node = 0, lut_record_size;	
	const U16_T *px_reg_list = NULL, *ledcurr_reg_list = NULL;
	int num_px_reg_list = 0, num_ledcurr_reg_list = 0;

	switch (panel_port) {
	case 0:
		flash_addr = FLASH_PART_OFFSET_PANEL_0_PATCH_BASE;
		magic_code = FLASH_PANEL_0_PATCH_MAGIC_CODE;
		break;

	case 1:
		flash_addr = FLASH_PART_OFFSET_PANEL_1_PATCH_BASE;
		magic_code = FLASH_PANEL_1_PATCH_MAGIC_CODE;
		break;

	default:
		EMSG("Invalid port\n");
		return -1;
	}

	// Read LUT header
	ret = rtiVC_ReadFLASH(flash_addr, (U8_T *)&phdr, sizeof(RD_LUT_HEADER_T));
	if (ret != 0) {
		EMSG("Fail to read the RDP patch header in FLASH\n");
		return -2;
	}

	if (phdr.magic_code == SWAP32(magic_code)) {
		num_flash_node = SWAP16(phdr.num_node);
		lut_node_size = num_flash_node * sizeof(RD_LUT_NODE_T);

		// Read LUT nodes
		rtiVC_ReadFLASH(flash_addr + sizeof(RD_LUT_HEADER_T),
						panel_buf + sizeof(RD_LUT_HEADER_T),
						lut_node_size);
	}
	else {
		if(phdr.magic_code != 0xFFFFFFFF) {
			EMSG("Magic code warning!\n");
			rtiVC_EraseSectorFLASH(flash_addr);
		}

		num_flash_node = 0;
	}

#if defined(CFG_PANEL_RDP551F)
	px_reg_list = rdp551f_pxshift_patch_reg_list;
	num_px_reg_list = ARRAY_SIZE(rdp551f_pxshift_patch_reg_list);

	ledcurr_reg_list = rdp551f_ledcurr_patch_reg_list;
	num_ledcurr_reg_list = ARRAY_SIZE(rdp551f_ledcurr_patch_reg_list);

#else
	#error "Code not present"
#endif

	panel_record = (RD_LUT_RECORD_T *)panel_buf;
	panel_node = &panel_record->node[0]; // Get the first position of node	

	if (ptype & VC_PANEL_PIXSEL_SHIFT) {
		num_flash_node = update_panel_flash_patch_data(panel_port, panel_node,
											num_flash_node, px_reg_list,
											num_px_reg_list);
	}

	if (ptype & VC_PANEL_LED_CURR) {
		num_flash_node = update_panel_flash_patch_data(panel_port, panel_node,
											num_flash_node, ledcurr_reg_list,
											num_ledcurr_reg_list);
	}

	if (ptype & VC_PANEL_MANUAL_PATCH) {
		num_flash_node = update_panel_flash_patch_data(panel_port, panel_node,
											num_flash_node, manual_rdp_patch_reg_list[panel_port],
											num_manual_rdp_patch_reg_list[panel_port]);
	}	

	/* Fill the panel header. */
	panel_record->hdr.num_node = SWAP16(num_flash_node);
	panel_record->hdr.magic_code = SWAP32(magic_code);

	lut_node_size = num_flash_node * sizeof(RD_LUT_NODE_T);
	lut_record_size = sizeof(RD_LUT_HEADER_T) + lut_node_size;

	return lut_record_size;
}

int rtiVC_DownloadPixelShiftPatchLUT(E_VC_PANEL_CTRL_PORT_T ctrl_port)
{
	int ret = 0;
	int patch_size;
	unsigned char *rdc_buf = NULL;
	unsigned char *panel_buf[MAX_NUM_VC_PANEL_PORT] = {NULL, NULL};

	rdc_buf = (unsigned char *)malloc(FLASH_PART_SIZE_RDC_PATCH);
	if (rdc_buf == NULL) {
		EMSG("RDC program buffer malloc error!\n");
		ret = -1;
		goto exit;
	}
	// Flash erased value.
	memset(rdc_buf, 0xFF, FLASH_PART_SIZE_RDC_PATCH);

	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		panel_buf[0] = (unsigned char *)malloc(FLASH_PART_SIZE_PANEL_PATCH);
		if (panel_buf[0] == NULL) {
			EMSG("RDP_0 program buffer malloc error!\n");
			ret = -2;
			goto exit;
		}
		// Flash erased value.
		memset(panel_buf[0], 0xFF, FLASH_PART_SIZE_PANEL_PATCH);
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		panel_buf[1] = (unsigned char *)malloc(FLASH_PART_SIZE_PANEL_PATCH);
		if (panel_buf[1] == NULL) {
			EMSG("RDP_1 program buffer malloc error!\n");
			ret = -3;
			goto exit;
		}
		// Flash erased value.
		memset(panel_buf[1], 0xFF, FLASH_PART_SIZE_PANEL_PATCH);
	}

	if (rdc200_pxshift_patch_reg_list) {
		/*
		 1. Prepare RDC patch data to program.
		*/
		patch_size = build_rdc_flash_patch_data(rdc_buf, VC_RDC_PIXSEL_SHIFT);
		if (patch_size < 0) {
			ret = -4;
			goto exit;
		}
	
		ret = rtiVC_SetupDownloadItem(FLASH_PART_OFFSET_RDC_PATCH_BASE, rdc_buf,
										patch_size, 16, VC_LUT_CLASS_RDC_INIT);	
	}

	/*
	 2. Prepare RDP patch data to program.
	*/
	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		patch_size = build_panel_flash_patch_data(0, panel_buf[0],
												VC_PANEL_PIXSEL_SHIFT);
		if (patch_size < 0) {
			ret = -5;
			goto exit;
		}		

		ret = rtiVC_SetupDownloadItem(FLASH_PART_OFFSET_PANEL_0_PATCH_BASE,
										panel_buf[0], patch_size,
										16, VC_LUT_CLASS_PANEL_INIT);	
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		patch_size = build_panel_flash_patch_data(1, panel_buf[1],
												VC_PANEL_PIXSEL_SHIFT);
		if (patch_size < 0) {
			ret = -6;
			goto exit;
		}		

		ret = rtiVC_SetupDownloadItem(FLASH_PART_OFFSET_PANEL_1_PATCH_BASE,
										panel_buf[1], patch_size,
										16, VC_LUT_CLASS_PANEL_INIT);	
	}

	if (ret == 0)
		rtiVC_StartDownload();
	else
		rtiVC_RemoveAllSetupItem();

	return 0;

exit:
	if (rdc_buf)
		free(rdc_buf);

	if (panel_buf[0])
		free(panel_buf[0]);

	if (panel_buf[1])
		free(panel_buf[1]);

	return ret;
}

int rtiVC_DownloadLedDutyPatchLUT(void)
{
	int ret = 0;
	int patch_size;
	unsigned char *rdc_buf = NULL;

	rdc_buf = (unsigned char *)malloc(FLASH_PART_SIZE_RDC_PATCH);
	if (rdc_buf == NULL) {
		EMSG("RDC program buffer malloc error!\n");
		ret = -1;
		goto exit;
	}
	// Flash erased value.
	memset(rdc_buf, 0xFF, FLASH_PART_SIZE_RDC_PATCH);

	/*
	 1. Prepare RDC patch data to program.
	*/
	patch_size = build_rdc_flash_patch_data(rdc_buf, VC_RDC_LED_DUTY);
	if (patch_size < 0) {
		ret = -4;
		goto exit;
	}

	ret = rtiVC_SetupDownloadItem(FLASH_PART_OFFSET_RDC_PATCH_BASE, rdc_buf,
									patch_size, 16, VC_LUT_CLASS_RDC_INIT);	

	if (ret == 0)
		rtiVC_StartDownload();
	else
		rtiVC_RemoveAllSetupItem();

	return 0;

exit:
	if (rdc_buf)
		free(rdc_buf);

	return ret;
}

int rtiVC_DownloadLedCurrentPatchLUT(E_VC_PANEL_CTRL_PORT_T ctrl_port)
{
	int ret = -99;
	int patch_size;
	unsigned char *panel_buf[MAX_NUM_VC_PANEL_PORT] = {NULL, NULL};

#if defined(CFG_PANEL_RDP501H)
	panel_buf[0] = (unsigned char *)malloc(FLASH_PART_SIZE_RDC_PATCH);
	if (panel_buf[0] == NULL) {
		EMSG("RDP501H program buffer malloc error!\n");
		return -1;
	}
	// Flash erased value.
	memset(panel_buf[0], 0xFF, FLASH_PART_SIZE_RDC_PATCH);	

	patch_size = build_rdc_flash_patch_data(panel_buf[0], 0);
	if (patch_size < 0) {
		free(panel_buf[0]);
		return -2;
	}

	ret = rtiVC_SetupDownloadItem(FLASH_PART_OFFSET_RDC_PATCH_BASE, panel_buf[0],
									patch_size, 16, VC_LUT_CLASS_RDC_INIT);	


#else
	if (ctrl_port & VC_PANEL_CTRL_PORT_0) {
		panel_buf[0] = (unsigned char *)malloc(FLASH_PART_SIZE_PANEL_PATCH);
		if (panel_buf[0] == NULL) {
			EMSG("RDP_0 program buffer malloc error!\n");
			return -1;
		}
		// Flash erased value.
		memset(panel_buf[0], 0xFF, FLASH_PART_SIZE_PANEL_PATCH);

		/* Prepare RDP patch data to program. */
		patch_size = build_panel_flash_patch_data(0, panel_buf[0],
												VC_PANEL_LED_CURR);
		if (patch_size < 0) {
			free(panel_buf[0]);
			return -2;
		}

		ret = rtiVC_SetupDownloadItem(FLASH_PART_OFFSET_PANEL_0_PATCH_BASE,
										panel_buf[0], patch_size,
										16, VC_LUT_CLASS_PANEL_INIT);
	}

	if (ctrl_port & VC_PANEL_CTRL_PORT_1) {
		panel_buf[1] = (unsigned char *)malloc(FLASH_PART_SIZE_PANEL_PATCH);
		if (panel_buf[1] == NULL) {
			EMSG("RDP_1 program buffer malloc error!\n");
			rtiVC_RemoveAllSetupItem();
			return -3;
		}

		// Flash erased value.
		memset(panel_buf[1], 0xFF, FLASH_PART_SIZE_PANEL_PATCH);

		/* Prepare RDP patch data to program. */
		patch_size = build_panel_flash_patch_data(1, panel_buf[1],
												VC_PANEL_LED_CURR);
		if (patch_size < 0) {
			rtiVC_RemoveAllSetupItem();
			return -4;
		}		

		ret = rtiVC_SetupDownloadItem(FLASH_PART_OFFSET_PANEL_1_PATCH_BASE,
										panel_buf[1], patch_size,
										16, VC_LUT_CLASS_PANEL_INIT);		
	}
#endif

	if (ret == 0)
		rtiVC_StartDownload();
	else
		rtiVC_RemoveAllSetupItem();

	return 0;
}

int rtiVC_DumpRdcPatchLut(RD_LUT_NODE_T lut_node_buf[], int num_buf_node)
{
	int ret;
	int num_flash_node = 0;
	RD_LUT_HEADER_T phdr;
	U32_T lut_node_size; /* The size of LUT node in bytes. */

	// Read LUT header
	ret = rtiVC_ReadFLASH(FLASH_PART_OFFSET_RDC_PATCH_BASE,
							(U8_T *)&phdr, sizeof(RD_LUT_HEADER_T));
	if (ret != 0) {
		EMSG("Fail to read the RDC patch header in FLASH\n");
		return -1;
	}

	if (phdr.magic_code != SWAP32(FLASH_RDC_PATCH_MAGIC_CODE)) {
		if(phdr.magic_code == 0xFFFFFFFF) {
			IMSG("Empty RDC patch\n");
			return 0; // Empty patch
		
}
		else { // if (phdr.magic_code != 0xFFFFFFFF) {
			EMSG("Magic code warning!. Need to erase RDC patch area\n");
			return -2;
		}
	}

	num_flash_node = SWAP16(phdr.num_node);
	lut_node_size = num_flash_node * sizeof(RD_LUT_NODE_T);

	if (num_buf_node < num_flash_node) {
		EMSG("Read buffer too small. The # of flash node is %d\n", num_flash_node);
		return -3;
	}

	// Read LUT nodes
	rtiVC_ReadFLASH(FLASH_PART_OFFSET_RDC_PATCH_BASE + sizeof(RD_LUT_HEADER_T),
					(U8_T *)lut_node_buf, lut_node_size);

	return num_flash_node;
}

int rtiVC_DumpPanelPatchLut(int panel_port, RD_LUT_NODE_T lut_node_buf[], int num_buf_node)
{
	int ret;
	int num_flash_node = 0;
	RD_LUT_HEADER_T phdr;
	U32_T lut_node_size; /* The size of LUT node in bytes. */
	U32_T flash_addr, magic_code;

	switch (panel_port) {
	case 0:
		flash_addr = FLASH_PART_OFFSET_PANEL_0_PATCH_BASE;
		magic_code = FLASH_PANEL_0_PATCH_MAGIC_CODE;
		break;

	case 1:
		flash_addr = FLASH_PART_OFFSET_PANEL_1_PATCH_BASE;
		magic_code = FLASH_PANEL_1_PATCH_MAGIC_CODE;
		break;

	default:
		EMSG("Invalid port\n");
		return -1;
	}

	// Read LUT header
	ret = rtiVC_ReadFLASH(flash_addr, (U8_T *)&phdr, sizeof(RD_LUT_HEADER_T));
	if (ret != 0) {
		EMSG("Fail to read the Panel_%d patch header in FLASH\n", panel_port);
		return -1;
	}

	if (phdr.magic_code != SWAP32(magic_code)) {
		if(phdr.magic_code == 0xFFFFFFFF) {
			IMSG("Empty Panel_%d patch\n", panel_port);
			return 0; // Empty patch
		
}
		else { // if (phdr.magic_code != 0xFFFFFFFF) {
			EMSG("Magic code warning!. Need to erase Panel_%d patch area\n", panel_port);
			return -2;
		}
	}

	num_flash_node = SWAP16(phdr.num_node);
	lut_node_size = num_flash_node * sizeof(RD_LUT_NODE_T);

	if (num_buf_node < num_flash_node) {
		EMSG("Read buffer too small. The # of flash node is %d\n", num_flash_node);
		return -3;
	}

	// Read LUT nodes
	rtiVC_ReadFLASH(flash_addr + sizeof(RD_LUT_HEADER_T),
					(U8_T *)lut_node_buf, lut_node_size);

	return num_flash_node;
}

int rtiVC_DownloadRdcManualPatchLUT(void)
{
	int ret = 0;
	int patch_size;
	unsigned char *rdc_buf = NULL;

	if (num_manual_rdc_patch_reg_list == 0) {
		EMSG("The number of manual RDC patch LUT is zero\n");
		return -44;
	}

	rdc_buf = (unsigned char *)malloc(FLASH_PART_SIZE_RDC_PATCH);
	if (rdc_buf == NULL) {
		EMSG("RDC program buffer malloc error!\n");
		ret = -1;
		goto exit;
	}
	// Flash erased value.
	memset(rdc_buf, 0xFF, FLASH_PART_SIZE_RDC_PATCH);

	/*
	 1. Prepare RDC patch data to program.
	*/
	patch_size = build_rdc_flash_patch_data(rdc_buf, VC_RDC_MANUAL_PATCH);
	if (patch_size < 0) {
		ret = -4;
		goto exit;
	}

	ret = rtiVC_SetupDownloadItem(FLASH_PART_OFFSET_RDC_PATCH_BASE, rdc_buf,
									patch_size, 16, VC_LUT_CLASS_RDC_INIT);	

	if (ret == 0)
		rtiVC_StartDownload();
	else
		rtiVC_RemoveAllSetupItem();

	return 0;

exit:
	if (rdc_buf)
		free(rdc_buf);

	num_manual_rdc_patch_reg_list = 0;

	return ret;
}

int rtiVC_DownloadPanelManualPatchLUT(E_VC_PANEL_CTRL_PORT_T ctrl_port)
{
	int ret = -99;
	int patch_size;
	UINT_T port = 0;
	UINT_T chk_port = (UINT_T)ctrl_port;
	unsigned char *panel_buf[MAX_NUM_VC_PANEL_PORT] = {NULL, NULL};
	U32_T flash_addr[MAX_NUM_VC_PANEL_PORT] = {
		FLASH_PART_OFFSET_PANEL_0_PATCH_BASE,
		FLASH_PART_OFFSET_PANEL_1_PATCH_BASE
	};

	if (!num_manual_rdp_patch_reg_list[0] && !num_manual_rdp_patch_reg_list[1]) {
		EMSG("No more manual RDP0 and RDP1 patch LUT\n");
		return -1;
	}

	do {
		if (chk_port & 0x1) {
			if (num_manual_rdp_patch_reg_list[port]) {
				panel_buf[port] = (unsigned char *)malloc(FLASH_PART_SIZE_PANEL_PATCH);
				if (panel_buf[port] == NULL) {
					EMSG("RDP%d program buffer malloc error!\n", port);
					ret = -2;
					break;
				}
				// Flash erased value.
				memset(panel_buf[port], 0xFF, FLASH_PART_SIZE_PANEL_PATCH);

				/* Prepare RDP patch data to program. */
				patch_size = build_panel_flash_patch_data(port, panel_buf[port],
														VC_PANEL_MANUAL_PATCH);
				if (patch_size < 0) {
					ret = -3;
					break;
				}

				ret = rtiVC_SetupDownloadItem(flash_addr[port],
												panel_buf[port], patch_size,
												16, VC_LUT_CLASS_PANEL_INIT);
				if (ret)
					break;
			}
			else
				DMSG("The number of manual RDP%d patch LUT is zero\n", port);
		}

		port++;
		chk_port >>= 1;
	} while (chk_port);

	if (ret == 0)
		rtiVC_StartDownload();
	else
		rtiVC_RemoveAllSetupItem();

	num_manual_rdp_patch_reg_list[0] = 0;
	num_manual_rdp_patch_reg_list[1] = 0;

	return 0;
}

void rtiVC_ResetManualRdcPatch(void)
{
	num_manual_rdc_patch_reg_list = 0;
}

int rtiVC_TuneManualRdcPatch(U16_T reg, U8_T val)
{
	int i;

	RDC_REG_SET(reg, val);

	for (i = 0; i < num_manual_rdc_patch_reg_list; i++) {
		if (manual_rdc_patch_reg_list[i] == reg) {			
			DMSG("[RDC manual patch] Already exist! reg(0x%04X)\n", reg);
			return 0;
		}
	}

	manual_rdc_patch_reg_list[i] = reg;
	num_manual_rdc_patch_reg_list++;

	return 0;
}

void rtiVC_ResetManualPanelPatch(void)
{
	num_manual_rdp_patch_reg_list[0] = 0;
	num_manual_rdp_patch_reg_list[1] = 0;
}

int rtiVC_TuneManualPanelPatch(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							U16_T reg, U8_T val)
{
	int i;
	BOOL_T found = FALSE;
	UINT_T port = 0;
	UINT_T chk_port = (UINT_T)ctrl_port;

	do {
		if (chk_port & 0x1) {
			found = FALSE;
			
			RDP_REG_SET(port, reg, val);

			for (i = 0; i < num_manual_rdp_patch_reg_list[port]; i++) {
				if (manual_rdp_patch_reg_list[port][i] == reg) {
					DMSG("[Panel manual patch] Already exist! reg(0x%04X)\n", reg);
					found = TRUE;
					break;
				}
			}

			if (found == FALSE) {
				manual_rdp_patch_reg_list[port][i] = reg;
				num_manual_rdp_patch_reg_list[port]++;
			
			}
		}

		port++;
		chk_port >>= 1;
	} while (chk_port);

	return 0;
}


