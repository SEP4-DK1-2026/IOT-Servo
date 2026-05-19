#pragma once

extern volatile uint16_t wakeups;

void sleep_timer_init(void);
void sleep_interval(void);