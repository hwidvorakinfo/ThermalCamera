/*
 * displaylib.h
 *
 *  Created on: Dec 29, 2014
 *      Author: daymoon
 */

#ifndef INCLUDES_DISPLAYLIB_H_
#define INCLUDES_DISPLAYLIB_H_

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "delay.h"
#include "fonts.h"
#include <stdlib.h>

typedef struct
{
  volatile uint16_t Register;  /* LCD Index Write            Address offset 0x00 */
  volatile uint16_t Data;      /* LCD Data Write             Address offset 0x02 */
}LCD_TypeDef;

/* Definitions ***************************************************************/

/*
 * FSMC Memory Bank 1: 0x60000000 to 6FFFFFFF
 * NAND bank selections:
 *   SECTION NAME      HADDR[17:16]  ADDRESS RANGE
 *   Address section   1X            0x020000-0x03FFFF
 *   Command section   01            0x010000-0x01FFFF
 *   Data section      00            0x000000-0x0FFFF
 */

#define LCD_BASE            ((uint32_t) (0x60000000 | 0x0001FFFE))
#define LCD                 ((LCD_TypeDef *) LCD_BASE)

#define GDDRAM_PREPARE      0x0022  /* Graphic Display Data RAM Register. */

#define LCD_WHITE           0xF7DE
#define LCD_YELLOW          0xFFE0
#define LCD_ORANGE          0xFA20
#define LCD_RED             0xF800
#define LCD_BLUE            0x001F
#define LCD_BLACK           0x0000

#define LCD_GREY            0xF7DE
#define LCD_GREEN           0x07E0

#define UI_TYRKYS			0x5FFE
#define UI_ORANGE			0xFD80
#define UI_GREEN			0x4FED

#define DISPSIZEY			320
#define	DISPSIZEX			240

extern void Init_LCD(void);
extern void Clear_Screen(uint16_t color);
extern void Draw_Pixel(uint16_t x, uint16_t y, uint16_t color);
extern void Display_String(uint16_t x, uint16_t y, uint8_t *ptr,uint16_t color);
extern void Set_Font(sFONT *fonts);
extern void Draw_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
extern void Draw_Rectangle(uint16_t x, uint16_t y, uint8_t sizex, uint16_t sizey, uint16_t color);
extern uint16_t Get_FLIR_color(uint8_t ratio);

#endif /* INCLUDES_DISPLAYLIB_H_ */
