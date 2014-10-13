/*
 * scheduling.h - A basic scheduler.
 *
 *  Created on: Oct 12, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */
#ifndef SCHEDULING_H_
#define SCHEDULING_H_

#include <sys/types.h>

/* 1ms Scheduler Ticks */
#define TICK_FREQ		1000
#define APB1_FREQ		72000000
#define TIM2_FREQ		(APB1_FREQ)
#define TIM2_PRESCALER	720
#define TIM2_PERIOD		((TIM2_FREQ/TIM2_PRESCALER)/TICK_FREQ)

/* Create a quick boolean type */
typedef int boolean;
#define TRUE	1
#define FALSE	0

/*
 * A doubly linked list is a linked structure made of these.
 * In this implementation of a doubly linked list, the head of the list is an element in the
 * list. The head cannot be removed from the list.
 */
typedef struct DlList_s {
	struct DlList_s* next;
	struct DlList_s* prev;
} DlList;

/*
 * A head is just an element, but define this for readability.
 */
typedef DlList DlListHead;

void dlListInit(DlListHead* head);
boolean dlListIsEmpty(DlListHead* head);
void dlListInsertBefore(DlList* toInsert, DlList* dest);
void dlListInsertFirst(DlList* toInsert, DlListHead* head);
void dlListAppend(DlList* toInsert, DlListHead* head);
void dlListRemove(DlList* toRemove);
void dlListRemoveFirst(DlListHead* head);

/*
 * Useful macros for managing lists.
 */
/*
 * Loop over each element in a list. DO NOT REMOVE ANY ELEMENT!
 */
#define dl_list_for_each(elem, head)	\
	for((elem) = (head)->next; (elem) != (head); (elem) = (elem)->next)

/*
 * Loop over each element in a list in a manner that is safe for removing the current element.
 * DO NOT remove any element but the current.
 */
#define dl_list_for_each_safe(elem, next_save, head) \
	for((elem) = (head)->next, (next_save) = (elem)->next; (elem) != head; \
		(elem) = (next_save), (next_save) = (elem)->next)

#endif /* SCHEDULING_H_ */
