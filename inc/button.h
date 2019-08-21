/*
 * button.h
 *
 *  Created on: 25. 8. 2017
 *      Author: daymoon
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "scheduler.h"
#include "defs.h"
#include "services.h"

#define BUTTON_PORT		GPIOA
#define BUTTON_CLOCKS	RCC_AHB1Periph_GPIOA
#define BUTTON_PIN		GPIO_Pin_0

extern void button_config(void);


#endif /* BUTTON_H_ */
