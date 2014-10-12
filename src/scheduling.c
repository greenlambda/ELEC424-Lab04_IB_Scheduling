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

/*
 * Initialize the GPIO which controls the LED
 */
static void InitializeLEDs() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef gpioStructure;
	gpioStructure.GPIO_Pin = GPIO_Pin_5;
	gpioStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioStructure);

	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET); /* clear bit; LED on */
}

/*
 * Initialize the timer and set the clock to the external oscillator
 */
static void InitializeTimer() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef timerInitStructure;
	/* Let the period be 2000 for 1 Hz, then 72000000/2000 = 36000 */
	timerInitStructure.TIM_Prescaler = 36000;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	/* 0.5 Hz, or 1/2 of 2000, the period */
	timerInitStructure.TIM_Period = 1000;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &timerInitStructure);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

/*
 * Enable the interrupt for the timer (TIM2)
 */
static void EnableTimerInterrupt() {
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority = 1;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
}

/*
 * Main function.  Initializes the GPIO, Timers, and
 */
int main() {
	sysClkInit72Mhz();
	InitializeLEDs();
	InitializeTimer();
	EnableTimerInterrupt();

	/* Loop. Forever. */
	for (;;) {
	}
}

/*
 * Interrupt service routine.  This function handles toggling the LED
 * when the timer throws an update flag.
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
