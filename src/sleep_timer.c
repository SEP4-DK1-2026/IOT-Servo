#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/io.h>
#include "sleep_timer.h"
#include <avr/interrupt.h>
#include <stdio.h>

// Volatile kan ændre sig uden for det normale flow. Den bruger nemlig ISR.
volatile uint16_t wakeups = 0;

ISR(WDT_vect)
{
    wakeups++;
}

void sleep_timer_init(void)
{
    // MCU Status Register og Watchdog Timer Control Register
    // Vi fjerner flaget her, som siger at Watchdog har reset.
    MCUSR &= ~(1 << WDRF);

    // Gør sådan at man kan ændre på watchdog indstillingerne.
    WDTCSR |= (1 << WDCE) | (1 << WDE);

    // WDIE = Watchdog Interrupt Enable & Watchdog timer. WDP0 = 16ms, WDP1 = 32ms, WDP2 = 64ms, WDP3 = 128ms.
    // interrupt mode, ca 8 sek
    WDTCSR = (1 << WDIE) | (1 << WDP3) | (1<<WDP0);
}

void sleep_interval(void)
{
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    sleep_mode();   

    sleep_disable();
}
