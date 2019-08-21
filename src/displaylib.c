/*
 * displaylib.c
 *
 *  Created on: Dec 29, 2014
 *      Author: daymoon
 */

#include "displaylib.h"

void Write_Command(uint16_t reg, uint16_t data);
void Write_GDDRAM_Prepare(void);
void Write_Data(uint16_t data);
void Set_Cursor(uint16_t x, uint16_t y);
void Display_Char(uint16_t x, uint16_t y, uint8_t c, uint16_t color);
void Draw_Char(uint16_t x, uint16_t y, const uint16_t *c, uint16_t color);

static sFONT *Current_Font;

static uint16_t FLIR_rainbow[] = { 0x0000, 0x0001, 0x0802, 0x0803, 0x1003, 0x1004, 0x1805, 0x1806, 0x2007, 0x2008, 0x2808, 0x2809, 0x300A, 0x300B,
						   	   	   0x380C, 0x380D, 0x400D, 0x400E, 0x400F, 0x4810, 0x4811, 0x5012, 0x5012, 0x5813, 0x5814, 0x6014, 0x6014, 0x6814,
								   0x6814, 0x7014, 0x7014, 0x7814, 0x7814, 0x8014, 0x8034, 0x8053, 0x8872, 0x8890, 0x90AF, 0x90CE, 0x98ED, 0x990B,
								   0xA12A, 0xA149, 0xA968, 0xA966, 0xB185, 0xB1A4, 0xB9C3, 0xB9E1, 0xBA00, 0xC220, 0xC240, 0xCA60, 0xCA80, 0xD2A0,
								   0xD2C0, 0xDAE0, 0xDB00, 0xE320, 0xE340, 0xEB60, 0xEB80, 0xF3A0, 0xF3C0, 0xFBE0, 0xFC00, 0xFC00, 0xFC20, 0xFC40,
								   0xFC60, 0xFC80, 0xFCA0, 0xFCC0, 0xFCE0, 0xFD00, 0xFD20, 0xFD40, 0xFD60, 0xFD80, 0xFDA0, 0xFDC0, 0xFDE1, 0xFE03,
								   0xFE25, 0xFE46, 0xFE68, 0xFE8A, 0xFEAB, 0xFEAD, 0xFECF, 0xFEF0, 0xFF12, 0xFF34, 0xFF55, 0xFF77, 0xFF99, 0xFFBA,
								   0xFFDC, 0xFFFE};


void Init_LCD(void)
{
  /* Reset */
  GPIO_ResetBits(GPIOE, GPIO_Pin_1);
  delay_ms(30);
  GPIO_SetBits(GPIOE, GPIO_Pin_1);
  delay_ms(10);

  /* Display ON Sequence (data sheet page 72) */
  Write_Command(0x0007, 0x0021);
  Write_Command(0x0000, 0x0001);
  Write_Command(0x0007, 0x0023);
  Write_Command(0x0010, 0x0000);  /* Exit Sleep Mode */
  delay_ms(30);
  Write_Command(0x0007, 0x0033);

  /*
   * Entry Mode R11h = 6018h
   *
   * DFM1 = 1, DFM0 = 1 => 65k Color Mode
   * ID0 = 1, AM = 1    => the way of automatic incrementing
   *                       of address counter in RAM
   */
  Write_Command(0x0011, 0x6018);
  Write_Command(0x0002, 0x0600);  /* AC Settings */

  /* Initialize other Registers */

  /*
   * Device Output Control R01h = 2B3Fh
   *
   * REV = 1            => character and graphics are reversed
   * BGR = 1            => BGR color is assigned from S0
   * TB  = 1            => sets gate output sequence (see datasheet page 29)
   * MUX[8, 5:0]        => specify number of lines for the LCD driver
   */
  Write_Command(0x0001, 0x2B3F);
}

/*
 * Write to LCD RAM.
 */

void Write_Command(uint16_t reg, uint16_t data)
{
  LCD->Register = reg;
  LCD->Data = data;
}

/*
 * Prepares writing to GDDRAM.
 * Next coming data are directly displayed.
 */

void Write_GDDRAM_Prepare(void)
{
  LCD->Register = GDDRAM_PREPARE;
}

/*
 * Writes data to last selected register.
 * Used with function Write_GDDRAM_Prepare().
 */

void Write_Data(uint16_t data)
{
  LCD->Data = data;
}

void Clear_Screen(uint16_t color)
{
  volatile uint32_t i = 0;

  Set_Cursor(0, 0);

  i = 0x12C00;
  Write_GDDRAM_Prepare();
  while(i--)
  {
    Write_Data(color);
  }
}

void Set_Cursor(uint16_t x, uint16_t y)
{
  Write_Command(0x004E, x);
  Write_Command(0x004F, y);
}

void Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
	Set_Cursor(x, y);
	Write_GDDRAM_Prepare();
	Write_Data(color);
}

void Set_Font(sFONT *fonts)
{
	Current_Font = fonts;
}

void Display_String(uint16_t x, uint16_t y, uint8_t *ptr,uint16_t color)
{
	uint16_t refcolumn = y;

	/* Send the string character by character on LCD */
	while ((*ptr != 0) & (((refcolumn + 1) & 0xFFFF) >= Current_Font->Width))
	{
		/* Display one character on LCD */
		Display_Char(x, refcolumn, *ptr, color);
		/* Decrement the column position by 16 */
		refcolumn -= Current_Font->Width;
		/* Point on the next character */
		ptr++;
	}
}

void Display_Char(uint16_t x, uint16_t y, uint8_t c, uint16_t color)
{
	c -= 32;
	Draw_Char(x, y, &Current_Font->table[c * Current_Font->Height], color);
}

void Draw_Char(uint16_t x, uint16_t y, const uint16_t *c, uint16_t color)
{
	uint32_t index = 0, i = 0;

	for(index = 0; index < Current_Font->Height; index++)
	{
		for(i = 0; i < Current_Font->Width; i++)
		{
			if( ((((c[index] & ((0x80 << ((Current_Font->Width / 12 ) * 8 ) ) >> i)) == 0x00) && (Current_Font->Width <= 12)) || (((c[index] & (0x1 << i)) == 0x00)&&(Current_Font->Width > 12 )))  == 0x00)
			{
				Draw_Pixel(x, y-1-i, color);
			}
		}
		x++;
	}
}

void Draw_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint8_t yLonger = 0;
	int incrementVal, endVal;
	int shortLen = y2-y1;
	int longLen = x2-x1;

	if(abs(shortLen) > abs(longLen))
	{
		int swap = shortLen;
		shortLen = longLen;
		longLen = swap;
		yLonger = 1;
	}
	endVal = longLen;

	if(longLen < 0)
	{
		incrementVal =- 1;
		longLen =- longLen;
		endVal--;
	}
	else
	{
		incrementVal = 1;
		endVal++;
	}

	int decInc;

	if(longLen == 0)
	{
		decInc = 0;
	}
	else
	{
		decInc = (shortLen << 16) / longLen;
	}

	int j = 0, i = 0;

	if(yLonger)
	{
		for(i = 0;i != endVal;i += incrementVal)
		{
			Draw_Pixel(x1 + (j >> 16),y1 + i,color);
			j += decInc;
		}
	}
	else
	{
		for(i = 0;i != endVal;i += incrementVal)
		{
			Draw_Pixel(x1 + i,y1 + (j >> 16),color);
			j += decInc;
		}
	}
}

void Draw_Rectangle(uint16_t x, uint16_t y, uint8_t sizex, uint16_t sizey, uint16_t color)
{
	uint16_t pomy;

	for (uint16_t posy = 0; posy < sizey; posy++)
	{// kreslime postupne linky v ose y
		pomy = y+posy;
		for (uint16_t posx = 0; posx < sizex; posx++)
		{// kreslime linku v sirce obdelniku
			Draw_Pixel(x+posx, pomy, color);
		}
	}
}

uint16_t Get_FLIR_color(uint8_t ratio)
{// vstup je 0 - 100 a vystup je barva
	return FLIR_rainbow[ratio];
}
