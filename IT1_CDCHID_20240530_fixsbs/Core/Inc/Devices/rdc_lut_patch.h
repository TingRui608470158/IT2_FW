
#ifndef __RDC_LUT_PATCH_H__
#define __RDC_LUT_PATCH_H__

#include "rti_vc_api.h"
#if defined(CFG_MDC_RDC200)
	#include "rdc200_flash_partitions.h"
#elif defined(CFG_MDC_RDC200A)
	#include "rdc200a_flash_partitions.h"
#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifndef SWAP16
	#define SWAP16(a)                \
        ((((U16_T)(a) >> 8) & 0x00ff) |    \
         (((U16_T)(a) << 8) & 0xff00))
#endif

#ifndef SWAP32
	#define SWAP32(a)                    \
    	((((U32_T)(a) >> 24) & 0x000000ff) |   \
        (((U32_T)(a) >>  8) & 0x0000ff00) |   \
        (((U32_T)(a) <<  8) & 0x00ff0000) |   \
        (((U32_T)(a) << 24) & 0xff000000))
#endif

typedef enum {
    VC_RDC_PIXSEL_SHIFT = 0x1,
    VC_RDC_LED_DUTY = 0x2,
    VC_RDC_MANUAL_PATCH = 0x4,
    VC_RDC_ALL = 0x7
} E_VC_RDC_PATCH_T;

typedef enum {
    VC_PANEL_PIXSEL_SHIFT = 0x1,
    VC_PANEL_LED_CURR = 0x2,
    VC_PANEL_MANUAL_PATCH = 0x4,
    VC_PANEL_ALL = 0x7
} E_VC_PANEL_PATCH_T;

int rtiVC_DumpPanelPatchLut(int panel_port, RD_LUT_NODE_T lut_node_buf[], int num_buf_node);
int rtiVC_DumpRdcPatchLut(RD_LUT_NODE_T lut_node_buf[], int num_buf_node);

int rtiVC_DownloadLedDutyPatchLUT(void);

int rtiVC_DownloadLedCurrentPatchLUT(E_VC_PANEL_CTRL_PORT_T ctrl_port);

int rtiVC_DownloadPixelShiftPatchLUT(E_VC_PANEL_CTRL_PORT_T ctrl_port);

void rtiVC_ResetManualRdcPatch(void);
int rtiVC_TuneManualRdcPatch(U16_T reg, U8_T val);
int rtiVC_DownloadRdcManualPatchLUT(void);

void rtiVC_ResetManualPanelPatch(void);
int rtiVC_TuneManualPanelPatch(E_VC_PANEL_CTRL_PORT_T ctrl_port,
							U16_T reg, U8_T val);
int rtiVC_DownloadPanelManualPatchLUT(E_VC_PANEL_CTRL_PORT_T ctrl_port);

#ifdef __cplusplus
}
#endif

#endif /* __RDC_LUT_PATCH_H__ */

