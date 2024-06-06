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

#ifndef __RTI_VC_CONFIG_H__
#define __RTI_VC_CONFIG_H__

/*
 * I2C slave address for RDP. (7 bit)
 */
//#define _CFG_RDP_SLAVE_ADDR	    0x38 // RDP360H
//#define _CFG_RDP_SLAVE_ADDR	    0x39 // RDP550F-e
#define _CFG_RDP_SLAVE_ADDR	    0x18 // RDP700Q / RDP550F-f / RDP551F / RDP502H / RDP370F

/*
 * Determine if RDP was attached to PORT 0 or not.
 */
#define _CFG_RDP_ATTACHED_TO_PORT_0

/*
 * Determine if RDP was attached to PORT 1 or not.
 */
#define _CFG_RDP_ATTACHED_TO_PORT_1

#if (!defined(ECLIPSE_RCP) && !defined(__KERNEL__) && defined(__linux__) /* Linux application */)
	/*
	 * Determine whether or not RTIMD driver in Linux kernel is used.
	 */
	#define _CFG_USE_RTIMD_KERNEL_DRIVER

	/* Linux I2C bus number for RDC or RDP 0 */
	#define _CFG_I2C_BUS_NR_RDC_RDP0	6

	/* Linux I2C bus number for RDP 1 */
	#define _CFG_I2C_BUS_NR_RDC_RDP1	1
#endif

/*
 * Defines the step of frame rate in KHz.
 */
#define _CFG_FRAME_RATE_CTRL_STEP		1

/*
 * Defines the step of LED duty.
 */
#define _CFG_LED_DUTY_CTRL_STEP		0.5

/**
* Mutex macros. 
*/ 
#if (!defined(__KERNEL__) && defined(__linux__) /* Linux application */ || defined(__APPLE__))
	#include <sys/mman.h>
	#include <pthread.h>

	#define VC_MUTEX_DECLARE(lock)	pthread_mutex_t lock;
	#define VC_MUTEX_INIT(lock)		pthread_mutex_init(lock, NULL)
	#define VC_MUTEX_LOCK(lock)		pthread_mutex_lock(lock)
	#define VC_MUTEX_FREE(lock)		pthread_mutex_unlock(lock)
	#define VC_MUTEX_DEINIT(lock)	((void)0)
	#define VC_MUTEX_EXTERN(lock)	extern pthread_mutex_t lock

#elif defined(WINCE) || defined(WINDOWS) || defined(_WIN32)
	#include <windows.h>
	#include <winbase.h>

	#define VC_MUTEX_DECLARE(lock)	CRITICAL_SECTION lock;
	#define VC_MUTEX_INIT(lock)		InitializeCriticalSection(lock)
	#define VC_MUTEX_LOCK(lock)		EnterCriticalSection(lock)
	#define VC_MUTEX_FREE(lock)		LeaveCriticalSection(lock)
	#define VC_MUTEX_DEINIT(lock)	DeleteCriticalSection(lock)
	#define VC_MUTEX_EXTERN(lock)	extern CRITICAL_SECTION lock

#else
	// TODO
	#define VC_MUTEX_DECLARE(lock)
	#define VC_MUTEX_INIT(lock)		((void)0)
	#define VC_MUTEX_LOCK(lock)		((void)0)
	#define VC_MUTEX_FREE(lock)		((void)0)
	#define VC_MUTEX_DEINIT(lock)	((void)0)
	#define VC_MUTEX_EXTERN(lock)	((void)0)
#endif

/**
* Debug message Configuration
*	IMSG: Information Message
*	DMSG: Debug Message
*	EMSG: Error Message
*/
#ifdef CFG_VC_DEBUG_MSG
	
	/* Debug message for console application */
	#if defined(ECLIPSE_RCP)
		extern int (*_ERRMSG)(const char *fmt, ...); //for UI Noti
		#define IMSG(s, ...) do { printf(s, ##__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
		#define DMSG(s, ...) do { printf(s, ##__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
		#define EMSG(...) do { _ERRMSG(__VA_ARGS__); } while (0)

	#elif (defined(_WIN32)||defined(_WINDOWS)) && !defined(CFG_CONSOLE_APP)
		void WinMessageBox_I(char* pFormat, ... );
		void WinMessageBox_D(char* pFormat, ... );
		void WinMessageBox_E(char* pFormat, ... );
		void WinDebugMessage(char* pFormat, ... );
		
		//#define	IMSG	WinMessageBox_I
		//#define	DMSG	WinMessageBox_D
		#define	IMSG	WinDebugMessage
		#define	DMSG	WinDebugMessage		
		#define	EMSG	WinMessageBox_E

	#elif defined(CFG_CONSOLE_APP) || defined(_WIN32)	
		#define	IMSG	printf
		#define	DMSG	printf
		#define	EMSG	printf
	
	/* Debug message for android framework */
	#elif defined(ANDROID) || defined(__ANDROID__)
		#include <android/log.h>
		#define  LOG_TAG    "vcube_hal"

		#ifndef LOGI
			#define  LOGI(...)\
				__android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
		#endif

		#ifndef LOGE
			#define  LOGE(...)\
				__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
		#endif

		#define	IMSG	LOGI
		#define	DMSG	LOGI
		#define	EMSG	LOGE
    #elif defined(__APPLE__)
    #define    IMSG    printf
    #define    DMSG    printf
    #define    EMSG    printf

	#else
    #define    IMSG    printf
    #define    DMSG    printf
    #define    EMSG    printf
	#endif
#else
	#define IMSG(fmt, ...)	do {} while (0) ///
	#define DMSG(fmt, ...)	do {} while (0)
	#define EMSG(fmt, ...)	do {} while (0)
#endif

#if defined(_CFG_RDP_ATTACHED_TO_PORT_0) && !defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	#define NUM_VC_DEV_CHANNEL		1
#elif !defined(_CFG_RDP_ATTACHED_TO_PORT_0) && defined(_CFG_RDP_ATTACHED_TO_PORT_1)
	#define NUM_VC_DEV_CHANNEL		1
#else	
	#define NUM_VC_DEV_CHANNEL		2 // RDC and RDP port 1
#endif

#endif /* __RTI_VC_CONFIG_H__ */


