#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "stdint.h"

#define KEYBOARD_WITH_REPORT_ID	0
#define KEYBOARD_SEND_WITH_VLOUME	0

//#define HID_KEYBOARD_REPORT_DESC_SIZE              63U
//
#if KEYBOARD_WITH_REPORT_ID
#define HID_KEYBOARD_REPORT_DESC_SIZE              104U
#else
#define HID_KEYBOARD_REPORT_DESC_SIZE              63U
#endif

// https://github.com/IntergatedCircuits/HidReportDef

/** set this to 0/1 if to have led or not
 * it can be defined in the build configuraton symbol */
#ifndef  HID_LED_SUPPORT
#   define HID_LED_SUPPORT 0
#endif

 /**  define  VOLUME_REPORT  to tehr eport ID to use for volume or 0 if not support*/
#define  HID_MEDIA_REPORT  2
#ifdef   HID_MEDIA_REPORT
#   define HID_MEDIA_SIZE    25
#else
#   define HID_MEDIA_SIZE   0
#endif

#if HID_MEDIA_REPORT == 1
#     error "volume report can't be 1 already sued for stad report"
#endif

#if HID_LED_SUPPORT
#   define HID_LED_SIZE    18
#else
#   define HID_LED_SIZE    0
#endif

#define HID_REPORT_DESC_SIZE    (47+HID_MEDIA_SIZE+HID_LED_SIZE)

#define HID_MEDIA_PAUSE  0xB1 // pause
#define HID_MEDIA_RECORD  0xB3
#define HID_MEDIA_SCAN_NEXT 0xB5
#define HID_MEDIA_SCAN_PREV 0xB6
#define HID_MEDIA_STOP  0xB7
#define HID_MEDIA_EJECT 0xB8
#define HID_MEDIA_VOL_UP 0xE9
#define HID_MEDIA_VOL_DOWN 0xEA
#define HID_MEDIA_PLAY  0xCD // play/pause

typedef struct
{
#if KEYBOARD_WITH_REPORT_ID
	uint8_t REPORT_ID;
#endif
	uint8_t MODIFIER;
	uint8_t RESERVED;
	uint8_t KEYCODE1;
	uint8_t KEYCODE2;
	uint8_t KEYCODE3;
	uint8_t KEYCODE4;
	uint8_t KEYCODE5;
#if !KEYBOARD_WITH_REPORT_ID
	uint8_t KEYCODE6;
#endif
} keyboardHID;

//extern char HidKeyboardReportDescriptor[];
extern keyboardHID keyboardhid;
extern uint8_t DEV76XX_HID_KEYCODE[];

extern void sendHidKeycode(uint8_t keycode);

#endif	// __KEYBOARD_H
