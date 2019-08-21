#ifndef __FONTS_H
#define __FONTS_H

/* Includes ------------------------------------------------------------------*/
#include "types.h"

typedef struct _tFont
{    
  const uint16_t *table;
  uint16_t Width;
  uint16_t Height;
  
} sFONT;

extern sFONT Font16x24;
extern sFONT Font12x12;
extern sFONT Font8x12;
extern sFONT Font8x8;

#define FONT_16	Font16x24
#define FONT_12 Font12x12
#define FONT_10 Font8x12
#define FONT_8  Font8x8
#define LINE(x) ((x) * (((sFONT *)LCD_GetFont())->Height))

#endif /* __FONTS_H */
