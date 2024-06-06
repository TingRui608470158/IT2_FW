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

#ifndef __RTI_VC_DLOAD_H__
#define __RTI_VC_DLOAD_H__

#include "rti_vc_api.h"

#ifdef __cplusplus
extern "C"{
#endif

struct VLIST_T {
    struct VLIST_T *next;
    struct VLIST_T *prev;
};

/* Offset of structure member */
#define VOFFSET_OF(type, member)  	((size_t)(&((type *)0)->member))

/*
 * Get the struct for this entry.
 *    ptr - [in] the &struct head pointer
 *    type - [in] the type of the struct this is embedded in
 *    member - [in] the name of the list struct within the struct
 */
#define VCONTAINER_OF(ptr, type, member)\
	((type *)(((unsigned char *)(ptr)) - VOFFSET_OF(type, member)))

/*=============================================================================
*
* VC Doubly-linked list Macro and Functions. 
*
*============================================================================*/
#define VLIST_HEAD_INITIALIZER(name)    { &(name), &(name) }
    
#define VLIST_HEAD_DEFINE(name) \
    struct VLIST_T name = VLIST_HEAD_INITIALIZER(name)

#define VLIST_HEAD_INIT(head_ptr)     \
    do {                        \
        (head_ptr)->next = (head_ptr);   \
        (head_ptr)->prev = (head_ptr);   \
    } while (0)

/**
 * VLIST_ENTRY - Get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define VLIST_ENTRY(ptr, type, member) \
	VCONTAINER_OF(ptr, type, member)

/**
 * VLIST_FIRST_ENTRY - Get the first element from a list
 * @head_ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define VLIST_FIRST_ENTRY(head_ptr, type, member) \
	VCONTAINER_OF((head_ptr)->next, type, member)

/**
 * VLIST_FOREACH	-	iterate over a list
 * @head:	the head for your list.
 * @pos:	the &struct VLIST_T to use as a loop cursor.
 */
#define VLIST_FOREACH(head, pos) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * VLIST_FOREACH_REMOVAL - iterate over a list safe against REMOVAL of list entry
 * @head:	the head for your list.
 * @pos:	the &struct list_head to use as a loop cursor.
 * @tmp:	another &struct VLIST_T to use as temporary storage
 */
#define VLIST_FOREACH_REMOVAL(head, pos, tmp)	\
	for (pos = (head)->next, tmp = pos->next;	\
		pos != (head);							\
		pos = tmp, tmp = pos->next)


/*
 * Insert a new entry between two consecutive entries.
 */
static INLINE void VLIST_INSERT(struct VLIST_T *new_node,
								struct VLIST_T *prev,
								struct VLIST_T *next)
{
	next->prev = new_node;
	new_node->next = next;
	new_node->prev = prev;
	prev->next = new_node;
}

/*
 * Insert a new entry after the specified head.
 */
static INLINE void VLIST_INSERT_HEAD(struct VLIST_T *head,
								struct VLIST_T *new_node)
{
	VLIST_INSERT(new_node, head, head->next);
}

/*
 * Insert a new entry before the specified head.
 */
static INLINE void VLIST_INSERT_TAIL(struct VLIST_T *head,
									struct VLIST_T *new_node)
{
	VLIST_INSERT(new_node, head->prev, head);
}

/* Unlink the specified list. */
static INLINE void VLIST_REMOVE(struct VLIST_T *list)
{
	struct VLIST_T *prev = list->prev;
	struct VLIST_T *next = list->next;

	next->prev = prev;
	prev->next = next;
}

/*
 * Test whether a list is empty.
 * empty if TRUE,
 * otherwise FALSE.
 */
static INLINE BOOL_T VLIST_IS_EMPTY(const struct VLIST_T *head)
{
	return (head->next == head);
}

/*
 * Tests whether @list is the last entry in list @head
 * last if TRUE,
 * otherwise FALSE.
 */
static INLINE BOOL_T VLIST_IS_LAST(const struct VLIST_T *head,
						const struct VLIST_T *list)
{
	return (list->next == head);
}

static INLINE struct VLIST_T *VLIST_FIRST(const struct VLIST_T *head)
{
	if (head->next != head)
		return head->next;
	else
		return NULL;
}

typedef struct
{
	struct VLIST_T head;
	UINT_T cnt; /* list count */
} VC_DLOAD_ITEM_LIST_T;

typedef enum
{
	VC_DLOAD_STATE_NULL = 0,
	VC_DLOAD_STATE_INIT,
	VC_DLOAD_STATE_ERASE,
	VC_DLOAD_STATE_PROGRAM,
	VC_DLOAD_STATE_VERIFY
} E_VC_DLOAD_STATE_T;


typedef struct
{
	/* Link in the item list. */
	struct VLIST_T link;

	/* Base address of FLASH to be programmed. */
	U32_T pgm_addr;

	/* Buffer of data to be programmed. */
	U8_T *pgm_data;

	U16_T pgm_data_size;

	/*
	 To use the moving of progress step and the verifying of programmed data.
	*/
	U16_T pgm_unit_size;

	E_VC_LUT_CLASS_T lut_class;
} VC_DLOAD_ITEM_INFO_T;


void rtiVC_DeinitDownload(void);

void rtiVC_InitDownload(void);

#ifdef __cplusplus
}
#endif

#endif /* __RTI_VC_DLOAD_H__ */

