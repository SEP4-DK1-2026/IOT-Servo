#include "servodriver.h"
#include "windowopener.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include "servo.h"

void servodriver_init(){
    servo_init(PWM_NORMAL);
    servo_start();
}

void servodriver_change(float temp, bool hasRained, float wantedTemp){
    if(shouldOpen(temp, hasRained, wantedTemp)) {
        servo_setAngle(PWM_A, 45);
        servo_setAngle(PWM_B, 45);
    }
    else {
        servo_setAngle(PWM_A, 0);
        servo_setAngle(PWM_B, 0);
    }
}

void servodriver_reset() {
    servo_setAngle(PWM_A, 0);
    servo_setAngle(PWM_B, 0);
}