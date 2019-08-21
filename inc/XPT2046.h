#ifndef _TOUCHPANEL_H_
#define _TOUCHPANEL_H_

/* Includes ------------------------------------------------------------------*/
#include "displaylib.h"
#include "mcuperipherals.h"
#include "stm32f4xx.h"
#include "defs.h"
#include "stm32f4xx_conf.h"

#define NODRAW					0
#define DRAW					1
#define ONEPRESS				0
#define CONTINUOUS				1


/* Private typedef -----------------------------------------------------------*/
typedef	struct Point
{
   uint16_t x;
   uint16_t y;
} Coordinate;

typedef struct Matrix 
{						
	int32_t An;
	int32_t Bn;
	int32_t Cn;
	int32_t Dn;
	int32_t En;
	int32_t Fn;
	int32_t Divider;
} Matrix ;

#include "UI.h"

extern void XPT2046_Init(void);				// controller initialization
extern uint16_t XPT2046_Press(void);			// returns if a pressure is present
extern void Touch_panel_draw_pixel(uint8_t mode);
extern void Set_panel_touched(FlagStatus state);
extern FlagStatus Get_panel_touched(void);
extern void Display_touched(void);
extern void XPT_select(void);
extern void XPT_release(void);
extern uint8_t get_Touch_mode(void);
extern void set_Touch_mode(uint16_t mode);

#endif
