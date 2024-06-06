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

#ifndef __RTI_VC_TYPES_H__
#define __RTI_VC_TYPES_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>	
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#if !defined(__KERNEL__) && defined(__linux__) /* Linux application */
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/ioctl.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <linux/i2c.h>
	#include <linux/i2c-dev.h>

#elif defined(_WIN32)
	#include <windows.h>
	#include <crtdbg.h>
	#pragma warning(disable:4996) // Disable C4996 warning message. 
#endif

#ifndef NULL
	#define NULL    	0
#endif

#ifndef FALSE
	#define FALSE		0
#endif

#ifndef TRUE
	#define TRUE		1
#endif

typedef int					INT_T;
typedef unsigned int		UINT_T;
typedef long				LONG_T;
typedef unsigned long		ULONG_T;

typedef signed char			S8_T;
typedef unsigned char		U8_T;

typedef signed short		S16_T;
typedef unsigned short		U16_T;

typedef float          		F32_T;

typedef volatile U8_T		VU8_T;
typedef volatile U16_T		VU16_T;
typedef volatile ULONG_T	VULONG_T;

#if defined(__GNUC__)
	typedef signed long long	S64_T;
	typedef unsigned long long	U64_T;

	typedef signed int			S32_T;
	typedef unsigned int		U32_T;

	typedef int                 BOOL_T;

	typedef ULONG_T	ADDR_T; /* 32-bit CPU */

	#define _FILE_NAME_ 		__FILE__  // include full path

	#define INLINE				inline
	#define PACKED				
	#define GNUPACKED			__attribute__((packed))
	#define ALIGNED(byte) 		__attribute__((aligned(byte)))

#elif !defined(__GNUC__) && (defined(__arm__) || defined(__arm))
	typedef signed int			S32_T;
	typedef unsigned int		U32_T;

	typedef double				F64_T;
	typedef int 				BOOL_T;

	typedef U32_T	ADDR_T; /* ARM 32-bit CPU */

	#if (__ARMCC_VERSION >= 310000) /* RVCT 3.1 */
 		#define _FILE_NAME_	__MODULE__ /* only file name */
	#else
		#define _FILE_NAME_	__FILE__ /* full path name */
	#endif

	#define INLINE			__inline
	#define FORCE_INLINE 	__forceinline
	#define PACKED			__packed
	#define GNUPACKED
	#define ALIGNED(byte)	__align(byte)

#elif defined(_WIN32)
	typedef signed int			S32_T;
	typedef unsigned int		U32_T;

	typedef double				F64_T;
	typedef int 				BOOL_T;


	typedef signed __int64		S64_T; // %I64d
	typedef unsigned __int64	U64_T; // %I64u

#ifdef _WIN64
	typedef U64_T	ADDR_T; /* 64-bit CPU */
#else
	typedef ULONG_T	ADDR_T; /* 32-bit CPU */
#endif

	#define _FILE_NAME_		__FILE__ /* full path name */

	#define INLINE			__inline
	#define FORCE_INLINE 	__forceinline
	#define PACKED
	#define GNUPACKED
	#define ALIGNED(byte)

#elif defined(__C51__) || defined(__CX51__) || defined(__C251__)
	#define INLINE
	#define PACKED
	#define GNUPACKED
	#define ALIGNED(byte)

	#define _FILE_NAME_ 	__FILE__  // include full path

	typedef signed long		S32_T;
	typedef unsigned long	U32_T;
	typedef char 			BOOL_T;

	typedef U32_T 			ADDR_T; /* Size of generic pointer is 3 bytes. */

#else
	#error "Code not present"
#endif

typedef volatile U32_T		VU32_T;

/* Macro to align the address/size to the next the specified boundary. */
#define ALIGN_4(x)	(((x) + 3) & ~3)
#define ALIGN_8(x)	(((x) + 7) & ~7)
#define ALIGN_16(x) (((x) + 15) & ~15)
#define ALIGN_32(x) (((x) + 31) & ~31)
#define ALIGN_64(x) (((x) + 63) & ~63)
#define ALIGN_LONG(x)	(((x) + (sizeof(LONG_T)-1)) & ~(sizeof(LONG_T)-1))
#define ALIGN_ADDR(x)	(((x) + (sizeof(ADDR_T)-1)) & ~(sizeof(ADDR_T)-1))

/* n should power of 2. */
#define ALIGN_N(x, n)	(((x)+(n)-1UL) & ~((n)-1UL))

#ifndef MAX
	#define MAX(a, b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
	#define MIN(a, b)    (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
	#define ABS(x) 		 (((x) < 0) ? -(x) : (x))
#endif

#define PAD(label, byteSize)	U8_T pad_##label[byteSize]

#define LO8(w)      	(U8_T)(w)
#define HI8(w)         	(U8_T)(((U16_T)(w) & 0xFF00) >> 8)

#define LO16(d)        	(U16_T)(d)
#define HI16(d)        	(U16_T)((U32_T)(d) & 0xFFFF0000) >> 16)

#define LO32(q)        	(U32_T)(q)
#define HI32(q)        	(U32_T)((U64_T)(q) & 0xFFFFFFFF00000000) >> 32)

#ifndef DIV32
	#define DIV32(x)		((x) >> 5) /* Divide by 32 */
#endif

#ifndef MOD32
	#define MOD32(x)        ((x) & 31)
#endif


/* 	Macro to calculate the size of an array. */
#ifndef ARRAY_SIZE
	#define ARRAY_SIZE(_A) (sizeof(_A) / sizeof((_A)[0]))
#endif /* ARRAY_SIZE */

#ifdef __GNUC__
	#define DIR_DELIMITER	'/'
#else
	#define DIR_DELIMITER	'\\'
#endif

//#define IS_POWER_OF_2(n)	(((n) != 0) && (((n) & ((n)-1)) == 0))
#define IS_POWER_OF_2(n)	(((n) & ((n)-1)) == 0)

#define UPPERCASE(c) (((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c))
#define LOWERCASE(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + 0x20) : (c))

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define ROUND(x, y)	((x) / (y) + ((x) % (y) * 10 / (y) >= 5 ? 1 : 0))

/* Offset of structure member */
#define MOFFSET_OF(type, member)  	((size_t)(&((type *)0)->member))

/* Size of structure member */
#define MSIZE_OF(type, member ) 	sizeof(((type *)0)->member)

/* Get the size of bits from specified structure member. */
#define MBITSIZE(type, member)	(MSIZE_OF(type, member) * 8)

/* Up to 32-bits */
#define MBITMASK(type, member)	((0xFFFFFFFF) >> (32 - MBITSIZE(type, member)))

/*
 * CONTAINER_OF - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 */
#define CONTAINER_OF(ptr, type, member)\
    ((type *)(((unsigned char *)(ptr)) - MOFFSET_OF(type, member)))


/* Busy-wait delay */
void rtiVC_DelayUs(U32_T microseconds);
#define VC_U_DELAY(microseconds)	rtiVC_DelayUs(microseconds)

#define VC_M_DELAY(milliseconds)				\
	do {										\
		if (milliseconds <= 5)					\
			rtiVC_DelayUs((milliseconds)*1000);	\
		else {									\
			U32_T loop = milliseconds;			\
			while (loop--)						\
				rtiVC_DelayUs(1000);			\
		}										\
	} while (0)

#ifdef __cplusplus
}
#endif

#endif /* __RTI_VC_TYPES_H__ */


