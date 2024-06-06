
#include <user_hid_keyboard.h>
#include <usb_hid_keys.h>
#include "usb_device.h"
#include "usbd_hid.h"
/*
char HidKeyboardReportDescriptor[HID_KEYBOARD_REPORT_DESC_SIZE] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x05,                    //   RE PORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
		0xc0,                           // END_COLLECTION
		0x05, 0x0c, // USAGE_PAGE (Consumer Page)
		0x09, 0x01, // USAGE (Consumer Control)
		0xa1, 0x01, // COLLECTION (Application)
		0x09, 0xea, // USAGE (Volume Decrement)
		0x09, 0xe9, // USAGE (Volume Increment)
		0x09, 0xe2, // USAGE (Mute)
		0x09, 0xcd, // USAGE (Play/Pause)
		0x15, 0x00, // LOGICAL_MINIMUM (0)
		0x25, 0x01, // LOGICAL_MAXIMUM (1)
		0x95, 0x07, // REPORT_COUNT (4)
		0x75, 0x01, // REPORT_SIZE (1)
		0x81, 0x02, // INPUT (Data,Var,Abs)
		0x75, 0x01, // REPORT_SIZE (1)
		0x95, 0x01, // REPORT_COUNT (4)
		0x81, 0x03, // INPUT (Cnst,Var,Abs)
    0xc0                           // END_COLLECTION
};
*/

extern USBD_HandleTypeDef hUsbDeviceFS;

#if KEYBOARD_WITH_REPORT_ID
keyboardHID keyboardhid = {1,0,0,0,0,0,0,0};
#else
keyboardHID keyboardhid = {0,0,0,0,0,0,0,0};
#endif

//#define KEYCODE_UP				0x26 	//0x52
//#define KEYCODE_DOWN			0x28	//0x51
//#define KEYCODE_LEFT			0x25	//0x50
//#define KEYCODE_RIGHT			0x27	//0x4f
//#define KEYCODE_PLUS			0xaf	// Volume +
//#define KEYCODE_MINUS			0xae	// Volume -
//#define KEYCODE_ENTER			0x28

// map to dev76xx bits
// CH: HID key code mapping changed - 20220819
uint8_t DEV76XX_HID_KEYCODE[] = {
		KEY_F1, KEY_F2, KEY_F3 , KEY_F4 , KEY_F5 ,
#if !KEYBOARD_SEND_WITH_VLOUME
		KEY_F6, KEY_F7, KEY_F8 , 0, 0,
				0, 0, KEY_F9  , KEY_F10   , 0
#else
				KEY_ENTER, HID_MEDIA_VOL_UP   , HID_MEDIA_VOL_DOWN    , 0, 0,
				0, 0, HID_MEDIA_VOL_UP  , HID_MEDIA_VOL_DOWN    , 0
#endif
};

//uint8_t DEV76XX_HID_KEYCODE[] = {
//		KEY_UP, KEY_DOWN, KEY_LEFT , KEY_RIGHT , KEY_KPENTER ,
//#if !KEYBOARD_SEND_WITH_VLOUME
//		KEY_ENTER, KEY_KPPLUS   , KEY_KPMINUS   , 0, 0,
//				0, 0, KEY_KPPLUS  , KEY_KPMINUS   , 0
//#else
//				KEY_ENTER, HID_MEDIA_VOL_UP   , HID_MEDIA_VOL_DOWN    , 0, 0,
//				0, 0, HID_MEDIA_VOL_UP  , HID_MEDIA_VOL_DOWN    , 0
//#endif
//};

/*
char* motionStr1[] = {
        "Up", "Down", "Left", "Right", "Forward",
        "Backward", "Clockwise", "Counterclockwise"
};
char* motionStr2[] = {
        "Wave", "Hover", "Clockwise continuous", "Counterclockwise continuous"
};
 */



void sendHidKeycode(uint8_t keycode)
{
	if (keycode>0) {
		DPRINTF("sendHidKeycode: 0x%x", keycode);
		keyboardhid.MODIFIER = 0x00;
#if KEYBOARD_WITH_REPORT_ID
		if (keycode==HID_MEDIA_VOL_UP || keycode==HID_MEDIA_VOL_DOWN) {
			keyboardhid.REPORT_ID = 2;
		}
		else {
			keyboardhid.REPORT_ID = 1;
		}
#endif
		keyboardhid.KEYCODE1 = keycode;
		USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&keyboardhid, sizeof (keyboardhid));
		HAL_Delay (50);

		keyboardhid.MODIFIER = 0x00;  // shift release
		keyboardhid.KEYCODE1 = 0x00;  // release key
		keyboardhid.KEYCODE2 = 0x00;
		USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&keyboardhid, sizeof (keyboardhid));
		HAL_Delay (100);
	}
	else {
		DPRINTF("sendHidKeycode: unknown");
	}
}

