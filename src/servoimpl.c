/***************************************************
 * servoimpl.c
 *  Servo implementation for closing and opening a window
 *  Author:  Benjamin Hansen
 *  Date:    2026-05-15
 *  Project: SEP4-DK1-2026/IOT-Servo
 **************************************************/

#include "servoimpl.h"
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

void servodriver_change(float temp, bool goingToRain, float wantedTemp){
    if(shouldOpen(temp, goingToRain, wantedTemp)) {
        servo_setAngle(PWM_A, 0);
        servo_setAngle(PWM_B, 0);
    }
    else {
        servo_setAngle(PWM_A, 90);
        servo_setAngle(PWM_B, 90);
    }
}

void servodriver_reset() {
    servo_setAngle(PWM_A, 90);
    servo_setAngle(PWM_B, 90);
}