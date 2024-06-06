
#ifndef __RDC_PREPARE_DLOAD_H__
#define __RDC_PREPARE_DLOAD_H__

#include "rti_vc_config.h"
#include "rti_vc_api.h"
#ifndef ECLIPSE_RCP
#ifdef __cplusplus
extern "C"{
#endif
#endif

int prepare_bmp_lut(char *filename);
int prepare_font_lut(char *filename);
int prepare_palette_lut(char *filename);
int prepare_optic_dc_lut(char *filename);
int prepare_panel_vertical_correction_lut(char *filename);
int prepare_panel_horizontal_correction_lut(char *filename);
int prepare_firmware_config_file(char *filename);

int prepare_panel_gamma_vt(char *filename, char *filename_1, double gamma_val); // for tuning

int prepare_panel_gamma_lut(char *filename);
int prepare_panel_init_lut(char *filename);
int prepare_rdc_init_lut(char *filename);
int prepare_firmware_hex_file(char *filename);
#ifndef ECLIPSE_RCP
#ifdef __cplusplus
}
#endif
#endif
#endif /* __RDC_PREPARE_DLOAD_H__ */


