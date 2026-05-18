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

static void uart0_wait_tx_complete(void)
{
    fflush(stdout);
    while (!(UCSR0A & (1 << TXC0))) { }
    UCSR0A |= (1 << TXC0);
}


void sleep_interval(void)
{
    wakeups = 0;

    while (wakeups != 397) // 397 * 8 sek = 3176 sek = ca 53 min ogås fordi vores 
    // program køre andre ting 
    // i baggrunden tager det ca 13% mere tid at løbe watchdog timeren
    {
        printf("[SLEEP] Entering low-power mode (cycle %u/112)\n", wakeups);
        uart0_wait_tx_complete();

        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_mode(); // Slukker CPU
        // Disabler sleep, så man kan vække den igen.
        sleep_disable();
    }
}
