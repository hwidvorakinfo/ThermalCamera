/*
 * services.c
 *
 *  Created on: Dec 28, 2014
 *      Author: daymoon
 */

#include "services.h"

/* private prototypes */
void TouchHoldOff_service(void);

// extern
void OrangeLED_service(void)
{
	uint16_t orangeled_port;

	orangeled_port = GPIO_ReadOutputData(KIT_LED_PORT);
	if (orangeled_port & KIT_ORANGE_LED)
	{
		//GPIO_ResetBits(KIT_LED_PORT, KIT_GREEN_LED);				// green LED off
		GPIO_ResetBits(KIT_LED_PORT, KIT_ORANGE_LED);				// orange LED off
		//GPIO_ResetBits(KIT_LED_PORT, KIT_RED_LED);					// red LED off
		//GPIO_ResetBits(KIT_LED_PORT, KIT_BLUE_LED);					// blue LED off
	}
	else
	{
		//GPIO_SetBits(KIT_LED_PORT, KIT_GREEN_LED);				// green LED off
		GPIO_SetBits(KIT_LED_PORT, KIT_ORANGE_LED);					// orange LED off
		//GPIO_SetBits(KIT_LED_PORT, KIT_RED_LED);					// red LED off
		//GPIO_SetBits(KIT_LED_PORT, KIT_BLUE_LED);					// blue LED off
	}

	if (GPIO_ReadInputData(BUTTON_PORT) & BUTTON_PIN)
	{
		GPIO_SetBits(KIT_LED_PORT, KIT_GREEN_LED);					// green LED off
	}
	else
	{
		GPIO_ResetBits(KIT_LED_PORT, KIT_GREEN_LED);				// green LED off
	}
}

// extern
void Delay_service(void)
{
	Set_Delay_finished(DELAY_FINISHED);
}

// extern
void Touchpanel_service(void)
{
	if ((get_spi_semaphore() != (SPI_UNUSED)) && (get_spi_semaphore() != (SPI_TOUCHPANEL)))
	{
		delay_ms(1);								// cekej na uvolneni periferie
	}
	set_spi_semaphore(SPI_TOUCHPANEL);				// rezervuj si periferii nebo potvrd jeji rezervaci
	SPI_touch_panel_slow_init();

	if (XPT2046_Press())							// pokud je stale aktivni stisk, vyvola jeho analyzu a zavola dalsi sluzbu ja zkracovani stisku
	{
		Touch_panel_draw_pixel(NODRAW);				// nacte souradnice a nakresli bod, dle modu
		Display_touched();							// zavolej funkci pro analyzu stisku
		if (get_Touch_mode() == CONTINUOUS)
		{
			// pokud je pozadavek na kontinualni snimani, zaloz novou sluzbu
			if(Scheduler_Add_Task(Touchpanel_service, 0, 0) == SCH_MAX_TASKS)// one-shot task, bez opakovani, okamzite spusteny
			{
					// chyba pri zalozeni service
			}
		}
		Set_panel_touched(SET);
	}
	else
	{
		Set_panel_touched(RESET);					// dalsi stisk jiz neexistuje
	}

/*if(mp3player_getstatus() == SILENCE)			// pokud prave nehraje hudba, prehraj zvuk kliknuti
{
	set_spi_semaphore(SPI_CODEC);				// prepni na codec
	vs1053PlaySoundClip();
}*/
	set_spi_semaphore(SPI_UNUSED);					// uvolni periferii

/*
 Display_touched();								// zavolej funkci zpracovavajici stisk
*/

	if(Scheduler_Add_Task(TouchHoldOff_service, 20, 0) == SCH_MAX_TASKS)	// one-shot task, bez opakovani, spusteny po 20ms
	{
		// chyba pri zalozeni service
	}
}

// private
void TouchHoldOff_service(void)
{
	Set_panel_touched(RESET);						// nastav priznak toho, ze stisk byl jiz obslouzen
}

void CB_play_service(void)
{
	int32_t transfer_len = -1;
	static uint8_t this_service_semaphore = FREE;

	// blokovani vicenasobneho spusteni
	if (this_service_semaphore != FREE)
	{
		return;
	}
	// alokace semaforu
	this_service_semaphore = STARTED;

	GPIO_SetBits(DEBUG_PIN_PORT, DEBUG_PIN_1);
	//transfer_len = player_receive_data();
	GPIO_ResetBits(DEBUG_PIN_PORT, DEBUG_PIN_1);

	while (transfer_len <= 0)
	{
		// uvolneni semaforu a konec
		this_service_semaphore = FREE;
		return;
	}

	this_service_semaphore = FREE;
}

void Debug_buffer_service(void)
{
	uint8_t used;

#define HORIZ_LINE1				60
#define HORIZ_LINE2				180
#define VERT_LINE1				80
#define VERT_LINE2				240
#define UI_PLAYING_BACKGROUND	UI_TYRKYS
#define UI_PLAYING_LINE			LCD_ORANGE

	//if (UI_Get_mode() == MODE_PLAYING)				// stav bufferu vykresli jen pokud je aktivni PLAYING mode
	//{
		// standardni verze dle obsazeneho mista v bufferu
		//used = ((VERT_LINE2-VERT_LINE1-1)*cb_used_space()/CBSIZE)+VERT_LINE1+1;

		// debugovaci verze ukazujici stav periodickeho plneni
		//used = ((VERT_LINE2-VERT_LINE1-1)*cb_get_tail_index()/CBSIZE)+VERT_LINE1+1;

		used = 10;

		Draw_Line(HORIZ_LINE2-1, VERT_LINE1+1, HORIZ_LINE2-1, VERT_LINE2-1, UI_PLAYING_BACKGROUND);		// vymaz stav buferu
		Draw_Line(HORIZ_LINE2-1, VERT_LINE1+1, HORIZ_LINE2-1, used, UI_PLAYING_LINE);					// stav buferu
	//}
}

void Temperaturemap_service(void)
{
	/*
	Amg8833_Process256();								// nacteni dat
	Amg8833_Interpolate256();							// dopocitej hodnoty v mape
	Amg8833_Scan(256);									// analyza nactenych dat
	Amg8833_Draw_screen256();								// vykresli teplotni mapu
	*/

	Amg8833_Process(MODE1024);							// nacteni dat
	Amg8833_Interpolate1024();							// dopocitej hodnoty v mape
	Amg8833_Scan(IRDIMX1024*IRDIMY1024);				// analyza nactenych dat
	Amg8833_Draw_screen1024();							// vykresli teplotni mapu
}

void Scale_service(void)
{
	// vykresli skalu
	Amg8833_Draw_scale_values();
	//Amg8833_CalculateCOM(MODE1024);					// prestalo fungovat pri 1024 - nutno opravit
}
