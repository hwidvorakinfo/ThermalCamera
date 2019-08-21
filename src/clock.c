/*
 * clock.c
 *
 *  Created on: Dec 28, 2014
 *      Author: daymoon
 */

/* Includes ------------------------------------------------------------------*/
#include "clock.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

RCC_ClocksTypeDef RCC_Clocks;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void Clock_config(void)
{
	/*!< At this stage the microcontroller clock setting is already configured,
	     this is done through SystemInit() function which is called from startup
	     files (startup_stm32f40_41xxx.s/startup_stm32f427_437xx.s/
	     startup_stm32f429_439xx.s/startup_stm32f401xx.s) before to branch to
	     application main. To reconfigure the default setting of SystemInit()
	     function, refer to system_stm32f4xx.c file
	*/

	RCC_GetClocksFreq(&RCC_Clocks);
}
