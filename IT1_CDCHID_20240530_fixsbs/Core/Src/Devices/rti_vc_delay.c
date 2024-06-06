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

#include "rti_vc_api.h"
#include "stm32l4xx.h"
#if defined(__APPLE__)
#include <unistd.h>
#endif

void rtiVC_DelayUs(U32_T microseconds)
{
#if 0
	// Porting here!

#if (!defined(__KERNEL__) && defined(__linux__) /* Linux application */ || defined(__APPLE__) )
	usleep(microseconds);
	
#elif defined(_WIN32)
	/* NOTE: Windows can't sleep for less than a millisecond. */
	DWORD cnt;

	cnt = DIV_ROUND_UP(microseconds, 1000);
	Sleep(cnt);
	
#else
	#error "Code not present"
#endif
#else
	uint32_t startval,tickn,delays,wait;

	startval = SysTick->VAL;
	tickn = HAL_GetTick();
	//sysc = 72000;  //SystemCoreClock / (1000U / uwTickFreq);
	delays = microseconds * 72; //sysc / 1000 * udelay;
	if(delays > startval)
	{
		while(HAL_GetTick() == tickn)
		{

		}
		wait = 72000 + startval - delays;
		while(wait < SysTick->VAL)
		{

		}
	}
	else
	{
		wait = startval - delays;
		while(wait < SysTick->VAL && HAL_GetTick() == tickn)
		{

		}
	}
#endif
}

