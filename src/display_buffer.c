/*
 * display_buffer.c
 *
 *  Created on: Jan 25, 2015
 *      Author: daymoon
 */

#include "display_buffer.h"

static uint8_t first_buffer_line;
static uint8_t last_buffer_line;
static sBUFFERLINE display_buffer[MAXBUFFERLINES];				// cele pole 24 prvku
static uint8_t update_display = NOACTION;

void Display_buffer_init(void)
{
	// inicializace spojiteho seznamu radku bufferu
	uint8_t index;
	sBUFFERLINE *pointer = &display_buffer[0];				//

	for (index = 0; index < MAXBUFFERLINES; index++)
	{
		pointer[index].text[0] = '\0';						// retezec je vsude prazdny
	}

	first_buffer_line = 0;				 					// inicializace klicovych ukazatelu
	last_buffer_line = 0;

	// pouziti bufferu
//	Clear_Screen(LCD_BLACK);
//	Display_add_to_buffer((uint8_t *)&message);
//	Display_buffer();
}

uint8_t Display_add_to_buffer(uint8_t *message)
{
	// prida dalsi radek do bufferu, pokud je misto, jinak systemem kruhoveho bufferu prepise nejstarsi zpravu

//#define OK	0
#define NOK		!OK

	uint8_t result = 0;
	uint8_t index;
	uint8_t found = NOK;
	sBUFFERLINE *temp_pointer;
	uint8_t message_to_be_added[CHARSONBUFFERLINE];

	// pripraveni lokalni kopie zpravy
	strcpy((char *)&message_to_be_added[0], (const char *)message);

	temp_pointer = &display_buffer[0];		// zacne prohledavat volne misto v bufferu
	for (index = 0; index < MAXBUFFERLINES; index++)
	{
		if (temp_pointer[(first_buffer_line + index) % MAXBUFFERLINES].text[0] == '\0')			// nasel volne misto v bufferu
		{
			found = OK;
			break;											// ukonci cyklus, v indexu nalezene misto
		}
	}
	if (found == NOK)										// nenasel volne misto, buffer je plny
	{
		first_buffer_line = ((first_buffer_line + 1) % MAXBUFFERLINES);	// nastavi novy prvni radek - prepisovani
		index = last_buffer_line;							// sem se bude ukladat obsah noveho radku
	}
	strcpy((char *)temp_pointer[index].text, (const char *)&message_to_be_added[0]);
	last_buffer_line = ((last_buffer_line + 1) % MAXBUFFERLINES);	// nastavi novy posledni radek
	//result = (uint8_t)StrLen((uint8_t *)temp_pointer->text);

	update_display = REDRAW;
	Display_buffer();

	return OK;
}

void Display_buffer(void)
{
	uint8_t x_text = 0;											// vypis bude od zacatku
	uint8_t index;
	int8_t number_of_lines;

	if (update_display != NOACTION)
	{
		Clear_Screen(DISPLAY_BUFFER_BACKGROUND);				// vycisti plochu
		Set_Font(&DISPLAY_BUFFER_FONT);							// nastavi font

		// zobrazi cely buffer of prvniho aktivniho radku do posledniho
		number_of_lines = last_buffer_line - first_buffer_line;
		if (number_of_lines <= 0)								// plny buffer a dochazi k prepisovani
		{
			number_of_lines = MAXBUFFERLINES;					// nech zobrazit tedy vse
		}

		index = 0;
		for ( ;number_of_lines > 0; number_of_lines--)
		{
			Display_String(x_text, 320, (uint8_t *)&display_buffer[(first_buffer_line+index)%MAXBUFFERLINES].text, DISPLAY_BUFFER_LOG);
			index++;
			x_text += (uint8_t)DISPLAY_BUFFER_FONT.Height + 2;
		}
		update_display = NOACTION;
	}
}
