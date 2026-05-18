/***************************************************
 * servodriver.h
 *  Servo interface for closing and opening a window
 *  Author:  Benjamin Hansen
 *  Date:    2026-05-15
 *  Project: SEP4-DK1-2026/IOT-Servo
 **************************************************/

#pragma once
#include <stdbool.h>

void servodriver_init();
void servodriver_change(float temp, bool goingToRain, float wantedTemp);
void servodriver_reset();