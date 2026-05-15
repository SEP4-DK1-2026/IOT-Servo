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
// #include "adxl345.h"

#include "servodriver.h"
#include "stdbool.h"

int main(void)
{
    sei();

    wifi_init();
    button_init();
    display_init();

    servodriver_init();

    float wantedTemperature = 0;
    bool hasRained = false;
    float temp = 0;

    bool turnedOn = true;
    bool lastButton1 = false;
    bool lastButton2 = false;
    bool lastButton3 = false;

while (1)
{
    bool button1 = button_get(1);
    bool button2 = button_get(2);
    bool button3 = button_get(3);

    if(turnedOn){
        if (button1 && !lastButton1) {
            wantedTemperature++;
        }

        if (button2 && !lastButton2) {
            wantedTemperature--;
        }

        servodriver_change(temp, hasRained, wantedTemperature);
    }

    if (button3 && !lastButton3)
    {
        turnedOn = !turnedOn;
        servodriver_reset();
    }

    lastButton1 = button1;
    lastButton2 = button2;
    lastButton3 = button3;

    turnedOn ? display_int(wantedTemperature) : display_setValues(16, 16, 16, 16);
}
}