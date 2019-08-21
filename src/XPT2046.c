/* Includes ------------------------------------------------------------------*/
#include "XPT2046.h"

static FlagStatus panel_touched = RESET;
static Coordinate display_coordinates;
static uint16_t touchmode = ONEPRESS;

/* Private variables ---------------------------------------------------------*/

#define THRESHOLD 2
#define	CHX 	0x90
#define	CHY 	0xD0

Coordinate *Read_XPT2046(void);			// returns coordinates of the pressure
void XPT2046_GetAdXY(Coordinate *touchscreen);
void getDisplayPixels(Coordinate * display_pixels, Coordinate * touchscreen_points);
uint16_t read_IRQ(void);
void WR_CMD (uint16_t cmd);
uint16_t RD_AD(void);
uint16_t Read_X(void);
uint16_t Read_Y(void);
FlagStatus Get_panel_touched(void);

void XPT2046_Init(void) 
{ 
} 

uint16_t Read_X(void)  
{  
	uint16_t curr_X;

	//XPT_select();
	//Delay(1);
	WR_CMD(CHX);
	//Delay(1);
	curr_X=RD_AD();
	//XPT_release();

	return curr_X;
} 

uint16_t Read_Y(void)  
{  
	uint16_t curr_Y;

	//XPT_select();
	//Delay(1);
	WR_CMD(CHY);
	//Delay(1);
	curr_Y=RD_AD();
	//XPT_release();

	return curr_Y;
} 

void XPT2046_GetAdXY(Coordinate *touchscreen)
{ 
	XPT_select();
	touchscreen->x = Read_X();
	touchscreen->y = Read_Y();
	XPT_release();
} 
	
Coordinate *Read_XPT2046(void)
{
  static Coordinate screen, touchscreen;
  int16_t m0,m1,m2,temp[3];
  uint8_t count = 0;
  uint16_t buffer[2][9]={{0},{0}};
  
  do
  {		   
	  XPT2046_GetAdXY(&touchscreen);
	  buffer[0][count] = touchscreen.x;
	  buffer[1][count] = touchscreen.y;
	  count++;  
  }
  while ((!read_IRQ()) && (count < 9));

  if(count >= 9)
  {
	  /* Average X  */
	  temp[0] = (buffer[0][0]+buffer[0][1]+buffer[0][2])/3;
	  temp[1] = (buffer[0][3]+buffer[0][4]+buffer[0][5])/3;
	  temp[2] = (buffer[0][6]+buffer[0][7]+buffer[0][8])/3;

	  // differencies between samples
	  m0 = temp[0]-temp[1];
	  m1 = temp[1]-temp[2];
	  m2 = temp[2]-temp[0];

	  // absolute values of differencies
	  m0 = m0 > 0 ? m0 : (-m0);
	  m1 = m1 > 0 ? m1 : (-m1);
	  m2 = m2 > 0  ?m2 : (-m2);

	  //if( (m0 > THRESHOLD)  &&  (m1 > THRESHOLD)  &&  (m2 > THRESHOLD) )
	  //{
	  //	  return 0;									// if there is a bigger difference than THRESHOLD, return without valid coordinate
	  //}

	  if(m0 < m1)
	  {
		  if(m2 < m0)
		  {
			  screen.x=(temp[0]+temp[2])/2;			// m2 < m0 <= m1, take two smallest values
		  }
		  else
		  {
			  screen.x=(temp[0]+temp[2])/2;			// m0 < (m2,m1)
		  }
	  }
	  else											// m1 < m0
	  {
		  if(m2 < m1)
		  {
			  screen.x=(temp[0]+temp[2])/2;			// m2 < m1 <= m0
		  }
		  else
		  {
			  screen.x=(temp[1]+temp[2])/2;			// m1 < (m2,m0)
		  }
	  }

	  /* Average Y  */
	  temp[0]=(buffer[1][0]+buffer[1][1]+buffer[1][2])/3;
	  temp[1]=(buffer[1][3]+buffer[1][4]+buffer[1][5])/3;
	  temp[2]=(buffer[1][6]+buffer[1][7]+buffer[1][8])/3;

	  // differencies between samples
	  m0 = temp[0]-temp[1];
	  m1 = temp[1]-temp[2];
	  m2 = temp[2]-temp[0];

	  // absolute values of differencies
	  m0 = m0 > 0 ? m0:(-m0);
	  m1 = m1 > 0 ? m1:(-m1);
	  m2 = m2 > 0 ? m2:(-m2);

	  //if( (m0 > THRESHOLD) && (m1 > THRESHOLD) && (m2 > THRESHOLD))
	  //{
	  //	  return 0;								// if there is a bigger difference than THRESHOLD, return without valid coordinate
	  //}

	  if (m0 < m1)
	  {
		  if(m2 < m0)
		  {
			  screen.y=(temp[0]+temp[2])/2;		// m2 < m0 < m1
		  }
		  else
		  {
			  screen.y=(temp[0]+temp[1])/2;		// m0 < (m2,m1)
		  }
	  }
	  else
	  {
		  if(m2 < m1)
		  {
			  screen.y=(temp[0]+temp[2])/2;		// m2 < m1 < m0
		  }
		  else
		  {
			  screen.y=(temp[1]+temp[2])/2;		// m1 < (m2,m0)
		  }
	  }
	  return &screen;
  }  
  return 0; 
}
	 
void getDisplayPixels(Coordinate * display_pixels, Coordinate * touchscreen_points)
{
	int32_t tempx;
	int32_t tempy;

	tempx = ((-touchscreen_points->x * 64) + (250097))/1000;
	tempy = ((-touchscreen_points->x * 1)  + (-touchscreen_points->y * 88) + 347913)/1000;
	display_pixels->x = tempx;
	display_pixels->y = tempy;
}

void XPT_release(void)
{
	GPIO_SetBits(TP_PORT, TP_CS);
}

void XPT_select(void)
{
	GPIO_ResetBits(TP_PORT, TP_CS);
}

uint16_t read_IRQ(void)
{
	return GPIO_ReadInputDataBit(TP_PORT, TP_IRQ);
}

uint16_t XPT2046_Press(void)
{
	return !read_IRQ();
}

void WR_CMD(uint16_t cmd)
{ 
	Touch_panel_ReadWrite(cmd);
} 

uint16_t RD_AD(void)
{ 
  uint16_t buf, temp; 

  temp = Touch_panel_ReadWrite(0x00);
  buf = temp << 8;
  //Delay(1);
  temp = Touch_panel_ReadWrite(0x00);

  buf |= temp; 
  buf >>= 3;
  buf &= 0xfff;

  return buf; 
}

void Touch_panel_draw_pixel(uint8_t mode)
{
	Coordinate touchscreen_coordinates;
	Coordinate *averaged_coordinates;

	averaged_coordinates = Read_XPT2046();
	if (averaged_coordinates != 0)								// returned valid coordinates?
	{
		touchscreen_coordinates.x = averaged_coordinates->x;
		touchscreen_coordinates.y = averaged_coordinates->y;

		/// novy display ma tyto otocene souradnice
		///touchscreen_coordinates.x = averaged_coordinates->y;
		///touchscreen_coordinates.y = averaged_coordinates->x;


		//getDisplayPoint(&display_coordinates, &touchscreen_coordinates, &matrix);
		getDisplayPixels(&display_coordinates, &touchscreen_coordinates);

		if (mode == DRAW)
		{
			//Draw_Pixel(240-display_coordinates.x, 320-display_coordinates.y, LCD_WHITE);
			Draw_Pixel(display_coordinates.x, display_coordinates.y, LCD_WHITE);
		}
	}
}

void Set_panel_touched(FlagStatus state)
{
	panel_touched = state;									// zprava o stisku
}

FlagStatus Get_panel_touched(void)
{
	return panel_touched;
}

void Display_touched(void)
{
	if (Get_panel_touched() == SET)
	{
		// Funkce pro obsluhu UI
		Display_touched_analyse(&display_coordinates);
	}
}

uint8_t get_Touch_mode(void)
{
	return touchmode;
}

void set_Touch_mode(uint16_t mode)
{
	touchmode = mode;
}


