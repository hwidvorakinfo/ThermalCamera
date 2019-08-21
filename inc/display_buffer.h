/*
 * display_buffer.h
 *
 *  Created on: Jan 25, 2015
 *      Author: daymoon
 */

#ifndef INCLUDES_DISPLAY_BUFFER_H_
#define INCLUDES_DISPLAY_BUFFER_H_

#include "displaylib.h"
#include "fonts.h"
#include "defs.h"
#include <stdio.h>
#include <string.h>

// definice DISPLAY_BUFFER povoli vypisy na displej
#define DISPLAY_BUFFER

#define MAXBUFFERLINES		24
#define CHARSONBUFFERLINE	41
#define	REDRAW				1
#define NOACTION 			0

#define DISPLAY_BUFFER_BACKGROUND	LCD_BLACK
#define DISPLAY_BUFFER_LOG			LCD_WHITE
#define DISPLAY_BUFFER_WARRNING		LCD_RED
#define DISPLAY_BUFFER_FONT			FONT_8

typedef struct _tBufferline
{
	uint8_t text[CHARSONBUFFERLINE];

} sBUFFERLINE;

extern void Display_buffer_init(void);
extern uint8_t Display_add_to_buffer(uint8_t *message);
extern void Display_buffer(void);


#endif /* INCLUDES_DISPLAY_BUFFER_H_ */
