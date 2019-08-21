/*
 * amg8833.h
 *
 *  Created on: 30. 8. 2017
 *      Author: daymoon
 */

#ifndef AMG8833_H_
#define AMG8833_H_

#include "stm32f4xx.h"
#include "defs.h"
#include "stm32f4xx_conf.h"
#include "displaylib.h"
#include "mcuperipherals.h"
#include "services.h"

#define SLAVE_ADDRESS 		0x68
#define IRARRAY				256

#define MODE256				25
#define MODE1024			102

// pocty ctvercu na obrazovce
#define IRDIMX				16
#define	IRDIMY				16
#define IRDIMX1024			29
#define	IRDIMY1024			29

// velikost pixelu na obrazovce
#define RECTSIZEX			15
#define RECTSIZEY			15
#define RECTSIZEX1024		8
#define RECTSIZEY1024		8

#define MAXPIXREAD			64
#define MAXSCANREAD			255
#define COMSIZEX			5
#define COMSIZEY			5
#define COMTHRESHOLD		26
#define COMMAXLIMIT			20

#define NUMBEROFTEMPSTEPS	6
#define NUMBEROFVALUES		255
#define MAXDEGREES			100
#define MINDEGREES			24
#define ZEROPOINT			1024

#define SCALESIZEX			2
#define SCALESIZEY			30
#define SCALEOFFSETX		20
#define SCALEOFFSETY		0
#define SCALETEMPSIZEX		10
#define SCALETEMPSIZEY		52
#define SCALETEMPMAXPOSX	10
#define SCALETEMPMAXPOSY	52
#define SCALETEMPMINPOSX	220
#define SCALETEMPMINPOSY

#define LASERPOSX			112
#define LASERPOSY			52

#define INTMODE0			0
#define INTMODE1			1
#define INTMODE2			2
#define INTMODEMAX			3

#define INTERPOSX			56
#define INTERPOSY			48
#define INTERTEXTPOSX		48
#define INTERTEXTPOSY		68

// registry AMG8833
#define FPSCREG				0x02
	#define FPS10			0x00
	#define FPS1			0x01

#define RESMAMOD			0x1F
#define MAMOD				0x07
	#define VAL1			0x50
	#define VAL2			0x45
	#define VAL3			0x57
	#define VAL4			0x00
	#define FLOATAVR		0x20

#define LASERACTIVE			1
#define LASERNONACTIVE		!LASERACTIVE

void Amg8833_Init(void);
void Amg8833_Process256(void);
void Amg8833_Process(uint8_t mode);
void Amg8833_Draw_screen256(void);
void Amg8833_Draw_screen1024(void);
void Amg8833_Scan(uint16_t size);
void Amg8833_Interpolate256(void);
void Amg8833_Interpolate1024(void);
void Amg8833_WriteCMD(uint8_t addr, uint8_t byte);
void Amg8833_Draw_scale_values(void);
void Amg8833_Scale_init(void);
void Amg8833_CalculateCOM(uint8_t mode);
void set_interpolation_mode(uint8_t mode);
uint8_t get_interpolation_mode(void);
void Amg8833_Interpolation_init(void);


void laser_init(void);
void laser_control(uint8_t mode);
void laser_icon_Draw();
void laser_icon_Clear();

#endif /* AMG8833_H_ */
