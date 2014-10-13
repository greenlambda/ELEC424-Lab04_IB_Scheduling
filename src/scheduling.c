/*
 * Name: scheduling.c
 * Authors: Jeremy Hunt and Christopher Buck
 * Date: 10-11-14
 * Description: A simple task scheduler for varying priority tasks.
 */

#include "scheduling.h"

#include "sys_clk_init.h"
#include "lab04_tasks.h"

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

/*
 * Initialize the GPIO which controls the LED
 */
static void initLeds() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef gpioStructure;
	gpioStructure.GPIO_Pin = GPIO_Pin_5;
	gpioStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioStructure);

	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET); /* clear bit; LED on */
}

/*
 * Initialize the timer to run at TICK_FREQ Hz (usually about 1000) so that we can be interrupted
 * regularly in order to update the task timers.
 */
static void initTickTimer() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef timerInitStructure;
	timerInitStructure.TIM_Prescaler = TIM2_PRESCALER;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = TIM2_PERIOD;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &timerInitStructure);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

/*
 * Enable the interrupt for the timer (TIM2)
 */
static void enableTickTimerInterupt() {
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority = 1;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
}

struct Task_s {
	DlList queueElem;
	void (*function)(void);
	unsigned int priority;
	unsigned int blockedTimer;
};

/*
 * Main function.  Initializes the GPIO, Timers, and
 */
DlListHead readyTasks;
int main() {
	sysClkInit72Mhz();
	initLeds();
	initTickTimer();
	enableTickTimerInterupt();

	/* Loop. Forever. */
	DlList* curTask;
	dl_list_for_each(curTask, &readyTasks) {

	}
	for (;;) {
		if (blockedTimer == 0) {
			detectEmergency();
			blockedTimer
		}
	}
}

/*
 * Interrupt service routine.
 * Function caller setup in "startup_stm32f10x_md.s"
 *
 * This routine updates the tick timer for each task.
 */
void TIM2_IRQHandler() {
	static unsigned char stateLED = 0;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); /* clear flag */
		if (stateLED)
			GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
		else
			GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET);
		stateLED = 1 - stateLED; /* flip the state for next operation */
	}
}


/*
 * This initializes a doubly linked list so that it is empty
 * (e.g. the head points to itself).
 */
void dlListInit(DlListHead* head) {
	head->next = head;
	head->prev = head;
}

/*
 * Returns TRUE iff the doubly linked list headed by head is empty. Head can actually be any
 * element in the list, because the list must always contain the head element, so if this is
 * called on anything but the head, the list must not be empty.
 */
boolean dlListIsEmpty(DlListHead* head) {
	if (head->next == head && head->prev == head) {
		return TRUE;
	} else {
		return FALSE;
	}
}


/*
 * Inserts an element before the given point in the doubly linked list.
 */
void dlListInsertBefore(DlList* toInsert, DlList* dest) {
	/* Link the new process to the dest. */
	toInsert->next = dest;
	toInsert->prev = dest->prev;
	/* Link the previous process to the new process */
	dest->prev->next = toInsert;
	/* Link the next process to the new process */
	dest->prev = toInsert;
}

/*
 * Helper function to insert an element to be the first of a list given the head.
 */
void dlListInsertFirst(DlList* toInsert, DlListHead* head) {
	dlListInsertBefore(toInsert, head->next);
}

/*
 * Helper function to insert an element at the end of a list with the given head.
 */
void dlListAppend(DlList* toInsert, DlListHead* head) {
	dlListInsertBefore(toInsert, head);
}

/*
 * Removes the element at the given point from a doubly linked list.
 *
 * Updates the given head if needed.
 */
void dlListRemove(DlList* toRemove) {
	/* Skip over the links to this process. */
	toRemove->prev->next = toRemove->next;
	toRemove->next->prev = toRemove->prev;
	/* Null the internal pointers. */
	toRemove->next = NULL;
	toRemove->prev = NULL;
}

/*
 * Helper function to remove the first element of a list given the head.
 */
void dlListRemoveFirst(DlListHead* head) {
	dlListRemove(head->next);
}


