/***************************************************
 * windowopener.c
 *  Implementation for when a window should open
 *  Author:  Benjamin Hansen
 *  Date:    2026-05-15
 *  Project: SEP4-DK1-2026/IOT-Servo
 **************************************************/

#include "windowopener.h"
#include "stdbool.h"

bool shouldOpen(float temp, bool goingToRain, float wantedTemp) {
    return temp <= wantedTemp || goingToRain ? false : true;
}