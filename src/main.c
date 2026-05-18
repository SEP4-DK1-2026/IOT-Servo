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
#include "button.h"
#include "uart_stdio.h"
#include "led.h"
#include "display.h"
#include "wifi.h"
#include "button.h"
#include "buzzer.h"
#include "servo.h"
#include "adc.h"
#include "tone.h"
#include "timer.h"
#include "network.h"

int main(void)
{
    sei();
    uart_stdio_init(115200);

    led_init();
    button_init();
    display_init();
    wifi_init();
    network_init();
    servo_init(PWM_NORMAL);
    servo_start();

    bool rain = false;
    float temp = 0.0f;

    // 3 forsøg på at checke efter vejret  med 5000 ms timeout fuck dig nigga
    if (network_check_weather(&rain, &temp, 3, 5000))
    {
        printf("[WEATHER] rain=%d temp=%.2f\n", rain ? 1 : 0, temp);

        if (rain)
        {
            // Eksempel: sæt servo til parkeringsposition ved regn
            servo_setAngle(PWM_A, 90);  
        }
        else
        {
            // Normal position
            servo_setAngle(PWM_A, 0);
        }
        if (temp < 10.0f)
        {
            printf("Temp below 10 degrees");
            servo_setAngle(PWM_A, 90);
        }
        else if (temp > 15.0f)
        {
            printf("temp above 15");
            servo_setAngle(PWM_A, 45);
        }
        else
        {
            servo_setAngle(PWM_A, 0);
        }
    }
    else
    {
        printf("[WEATHER] check failed after retries\n");
        // fallback handling (ingen ændring eller sikker tilstand)
    }
}