/*
 * switch_2d_3d.h
 *
 *  Created on: 2021年12月30日
 *      Author: LEAPSY
 */

#ifndef INC_SWITCH_VIEW_H_
#define INC_SWITCH_VIEW_H_

extern uint8_t CDCRxBuffer[64];
extern uint32_t cdc_rx_total;

/*
 * The boolean variable two_dim_video_mode_flag is true
 * when switching to 2D mode, otherwise 3D.
 */
extern bool two_dim_video_mode_flag;
/*
 * The boolean variable three_dim_video_mode_flag is true
 * when switching to 3D mode, otherwise 2D.
 */
extern bool three_dim_video_mode_flag;

/*
 * A 2D/3D switch GPIO pin that changes the view from
 * 2D to 3D and vice-versa (toggle).
 */
typedef enum
{
  Switch_2D_3D_Set    =	0,   /*!< Pin 13 pressed */
  Switch_2D_3D_Reset  =	1,   /*!< Pin 13 pressed again */
} SwitchState_TypeDef;

extern void VideoOutputEnable(void);
extern void VideoOutputDisable(void);
#endif /* INC_SWITCH_VIEW_H_ */
