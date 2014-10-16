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
static void ledsInit() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef gpioStructure;
	gpioStructure.GPIO_Pin = GPIO_Pin_5;
	gpioStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioStructure);

	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET); /* clear bit; LED on */
}

/*
 * Initialize the High Priority Timer to run at 10 ms
 */
static void tickHighPrioTimerInit() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseInitTypeDef timerInitStructure;
	timerInitStructure.TIM_Prescaler = TIM1_PRESCALER;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = TIM1_PERIOD;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &timerInitStructure);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM1, ENABLE);

	/* Initialize the interrupt for TIM1 */
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = TIM1_PRIO;
	nvicStructure.NVIC_IRQChannelSubPriority = TIM1_SUB_PRIO;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
}

/*
 * Initialize the Low Priority Timer to run at 500 ms
 */
static void tickLowPrioTimerInit() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef tim2Init;
	tim2Init.TIM_Prescaler = TIM2_PRESCALER;
	tim2Init.TIM_CounterMode = TIM_CounterMode_Up;
	tim2Init.TIM_Period = TIM2_PERIOD;
	tim2Init.TIM_ClockDivision = TIM_CKD_DIV1;
	tim2Init.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &tim2Init);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM2, ENABLE);

	/* Initialize the interrupt for TIM2 */
	NVIC_InitTypeDef tim2Nvic;
	tim2Nvic.NVIC_IRQChannel = TIM2_IRQn;
	tim2Nvic.NVIC_IRQChannelPreemptionPriority = 0;
	tim2Nvic.NVIC_IRQChannelSubPriority = 1;
	tim2Nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&tim2Nvic);
}


/*
 * Main function.  Initializes the GPIO, Timers, and
 */
int main() {
	sysClkInit72Mhz();
	ledsInit();
	tickHighPrioTimerInit();
	tickLowPrioTimerInit();

	/* Loop. Forever. */
	for (;;) {
		/* When we aren't doing stuff in interrupts, run the logger. */
		logDebugInfo();
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


