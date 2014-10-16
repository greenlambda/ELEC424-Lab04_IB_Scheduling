/*
 * scheduling.h - A basic scheduler.
 *
 *  Created on: Oct 12, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */
#ifndef SCHEDULING_H_
#define SCHEDULING_H_

#include <sys/types.h>

/* 10ms Scheduler Ticks */
#define HIGH_PRIO_TICK_FREQ		100
#define LOW_PRIO_TICK_FREQ		2
#define APB1_FREQ				72000000
#define APB2_FREQ				72000000

#define TIM1_FREQ				(APB2_FREQ)
#define TIM1_PRESCALER			720
#define TIM1_PERIOD				((TIM1_FREQ/TIM1_PRESCALER)/HIGH_PRIO_TICK_FREQ)
#define TIM1_PRIO				1
#define TIM1_SUB_PRIO			0

#define TIM2_FREQ				(APB1_FREQ)
#define TIM2_PRESCALER			36000
#define TIM2_PERIOD				((TIM2_FREQ/TIM2_PRESCALER)/LOW_PRIO_TICK_FREQ)
#define TIM2_PRIO				1
#define TIM2_SUB_PRIO			1

/* Create a quick boolean type */
typedef int boolean;
#define TRUE	1
#define FALSE	0

#endif /* SCHEDULING_H_ */
