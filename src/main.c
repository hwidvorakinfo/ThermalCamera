/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "main.h"
			

int main(void)
{
	int32_t retVal;

	Clock_config();
	Scheduler_init();									// inicializace kooperativniho scheduleru a jeho odstartovani
	Scheduler_start();									// nyni je jiz mozne zakladat tasky

	leds_config();										// konfigurace led na discovery desce a spusteni sluzeb pro led
	//button_config();									// konfigurace tlacitka - KOLIZE s JTAG
	Init_display_HW();									// konfigurace obrazove casti displeje
	Init_display_TP();									// konfigurace dotykove casti displeje

	#ifdef DISPLAY_BUFFER
	Display_buffer_init();								// ladici vypisy na displej
	//Display_add_to_buffer((uint8_t *)"...stm32f4 debug buffer...");
	Display_add_to_buffer((uint8_t *)"  AMG8833 Thermal camera 30x30px 2018");
	Debug_pins_init();
	#endif

	Amg8833_Init();										// konfigurace IR cidla

	while (1)
	{
		Scheduler_Dispatch_Tasks();						// sprava tasku - spousti tasky, maze neplatne tasky
	}
}
