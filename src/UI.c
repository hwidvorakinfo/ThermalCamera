/*
 * UI.c
 *
 *  Created on: Jan 2, 2015
 *      Author: daymoon
 */

#include "amg8833.h"
#include "UI.h"

#define MAX_NUM_OF_PRESSES	20

static Coordinate touched_history[MAX_NUM_OF_PRESSES];
static uint8_t history_index = 0;

static uint8_t laser_active = LASERNONACTIVE;

void Display_touched_analyse(Coordinate *coordinates)
{
	//	coordinates->x
	//	coordinates->y
	if (coordinates->y < SCALETEMPSIZEY)
	{
		if (laser_active == LASERACTIVE)
		{
			laser_active = LASERNONACTIVE;
			laser_icon_Clear();
		}
		else
		{
			laser_active = LASERACTIVE;
			laser_icon_Draw();
		}
		laser_control(laser_active);
	}
	else
	{
		set_interpolation_mode(get_interpolation_mode() + 1);
	}
}

uint8_t get_laser_active(void)
{
	return laser_active;
}
