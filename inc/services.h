/*
 * services.h
 *
 *  Created on: Dec 28, 2014
 *      Author: daymoon
 */

#ifndef INCLUDES_SERVICES_H_
#define INCLUDES_SERVICES_H_

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "leds.h"
#include "XPT2046.h"
#include "delay.h"
#include "displaylib.h"
#include "scheduler.h"
#include "mcuperipherals.h"
#include "button.h"
#include "amg8833.h"

#define MILISEKUND 	/1000

// periody jsou v milisekundach, neboli zakladni periode SysTick casovace
#define ORANGELED_SERVICE_PERIOD			(SCHEDULERPERIOD * 500 MILISEKUND)
#define TEMPERATUREMAP_PERIOD				(SCHEDULERPERIOD * 100 MILISEKUND)
#define SCALE_PERIOD						(SCHEDULERPERIOD * 100 MILISEKUND)

#define FREE			0
#define STARTED			127
#define REQUESTED		64


// sluzby
extern void OrangeLED_service(void);
extern void Delay_service(void);
extern void Touchpanel_service(void);
extern void wifi_service(void);
extern void CB_play_service(void);
extern void Debug_buffer_service(void);
extern void Temperaturemap_service(void);
extern void Scale_service(void);

#endif /* INCLUDES_SERVICES_H_ */
