/*****************************************************************************
 * main.c
 *  Main application file for the windowopener.
 *  This file delegates different actions, to different files.
 *  Push button 1 to increment the wanted outside temperature before the window
 *  should open, and button 2 for decrementing.
 *  It recieves weather information from an Azure Serverless Function, which
 *  gets the information from the databases.
 *
 *  Author:  Jonas Schwartz & Benjamin Hansen
 *  Date:    2026-05-15
 *  Project: SEP4-DK1-2026/IOT-Servo
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

#include "servodriver.h"
#include "stdbool.h"

int main(void)
{
    sei();
    uart_stdio_init(115200);

    wifi_init();
    network_init();
    button_init();
    display_init();
    servodriver_init();
  
    float wantedTemperature = 0;
    bool rain = false;
    float temp = 0.0f;

    bool turnedOn = true;
    bool lastButton1 = false;
    bool lastButton2 = false;
    bool lastButton3 = false;
  
    while(1) {
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
        
        network_check_weather(&rain, &temp, 3, 5000);

        servodriver_change(temp, rain, wantedTemperature);
      }
      
     if (button3 && !lastButton3) {
        turnedOn = !turnedOn;
        servodriver_reset();
      }
      
      lastButton1 = button1;
      lastButton2 = button2;
      lastButton3 = button3;
      
      turnedOn ? display_int(wantedTemperature) : display_setValues(16, 16, 16, 16);
    }
}