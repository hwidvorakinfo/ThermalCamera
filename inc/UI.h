/*
 * UI.h
 *
 *  Created on: Jan 2, 2015
 *      Author: daymoon
 */

#ifndef INCLUDES_UI_H_
#define INCLUDES_UI_H_

#include "XPT2046.h"

extern void Display_touched_analyse(Coordinate *coordinates);
uint8_t get_laser_active(void);

#endif /* INCLUDES_UI_H_ */
