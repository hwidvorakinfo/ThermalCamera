/*
 * button.c
 *
 *  Created on: 25. 8. 2017
 *      Author: daymoon
 */

#include "button.h"

void button_config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(BUTTON_CLOCKS, ENABLE);

	// button
	GPIO_InitStructure.GPIO_Pin = BUTTON_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BUTTON_PORT, &GPIO_InitStructure);
}
