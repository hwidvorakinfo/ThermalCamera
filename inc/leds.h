/*
 * leds.h
 *
 *  Created on: Dec 28, 2014
 *      Author: daymoon
 */

#ifndef INCLUDES_LEDS_H_
#define INCLUDES_LEDS_H_

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "scheduler.h"
#include "defs.h"
#include "services.h"

#define KIT_LED_PORT	GPIOD
#define KIT_LED_CLOCKS	RCC_AHB1Periph_GPIOD
#define KIT_GREEN_LED	GPIO_Pin_12
#define KIT_ORANGE_LED	GPIO_Pin_13
#define KIT_RED_LED		GPIO_Pin_14	// port pouzit v FSMC
#define KIT_BLUE_LED	GPIO_Pin_15 // port pouzit v FSMC

extern void leds_config(void);

#endif /* INCLUDES_LEDS_H_ */
