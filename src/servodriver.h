#pragma once
#include <stdbool.h>

void servodriver_init();
void servodriver_change(float temp, bool hasRained, float wantedTemp);
void servodriver_reset();