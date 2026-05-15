#pragma once
#include <stdint.h>
#include <stdbool.h>

bool network_check_weather(bool *out_rainNextHour,
                           float *out_temperature,
                           uint8_t retries,
                           uint32_t timeout_ms);

void network_init(void);