/*****************************************************************************
 * main.c
 *  Main application file for the IoT hardware drivers demo.
 *  This file initializes all the hardware drivers and demonstrates their
 *  functionality.
 *  Push button 2 on the shield during reset to enter continious sensor
 *  reading mode. Otherwise the program will run an interactive demo that
 *  allows you to test each driver individually by sending commands over UART.
 *  See interactive.c for details.
 *
 *  Author:  Erland Larsen
 *  Date:    2026-03-17
 *  Project: SPE4_API
 *****************************************************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "interactive.h"
#include "button.h"
#include "uart_stdio.h"
#include "led.h"
#include "pir.h"
#include "display.h"
#include "wifi.h"
#include "button.h"
#include "buzzer.h"
#include "dht11.h"
#include "proximity.h"
#include "servo.h"
#include "adc.h"
#include "light.h"
#include "soil.h"
#include "tone.h"
#include "timer.h"
// #include "adxl345.h"

int main(void)
{
    sei();

    led_init();
    button_init();
    display_init();
    proximity_init();
    light_init();
    soil_init(ADC_PK0);
    pir_init(pir_callback);
    //    tone_init();
    wifi_init();
    servo_init(PWM_NORMAL);
    //    adxl345_init();
    servo_start();

    while (1)
    {
        if (button_get(1))
        {
            servo_setAngle(PWM_A, 90);
            servo_setAngle(PWM_B, 90);
        }
        else if (button_get(2))
        {
            servo_setAngle(PWM_A, -90);
            servo_setAngle(PWM_B, -90);
        }
        else if (button_get(3))
        {
            servo_setAngle(PWM_A, 0);
            servo_setAngle(PWM_B, 0);
        }
    }
}