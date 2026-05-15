/***************************************************
 * windowopener.c
 *  Interface for when a window should open
 *  Author:  Benjamin Hansen
 *  Date:    2026-05-15
 *  Project: SEP4-DK1-2026/IOT-Servo
 **************************************************/

#pragma once
#include "stdbool.h"

bool shouldOpen(float temp, bool goingToRain, float wantedTemp);