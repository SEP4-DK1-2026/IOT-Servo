#include "network.h"
#include "wifi.h"
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

static char tcp_rx_buffer[700];
static volatile bool tcp_received = false;

// ================= CALLBACK =================
static void tcp_callback(void)
{
    tcp_received = true;
}

// ================= INIT =================
void network_init(void)
{
    wifi_init();
    _delay_ms(4000);

    wifi_command_disable_echo();
    wifi_command_set_mode_to_1();

    int attempts = 0;

    while (wifi_command_join_AP("Namnam", "Benjamin") != WIFI_OK)
    {
        _delay_ms(2000);
        attempts++;

        if (attempts >= 10)
        {
            break;
        }
    }

    if (attempts >= 10)
    {
        return; // VIGTIG: Giv op her!
    }

    wifi_command_set_to_single_Connection();
}

// ================= Simple JSON parser helpers =================
// Find key in buf and return pointer to character after ':' (or NULL)
static const char *json_find_key_value(const char *buf, const char *key)
{
    const char *p = strstr(buf, key);
    if (!p)
        return NULL;
    p = strchr(p, ':');
    if (!p)
        return NULL;
    p++; // move past ':'
    while (*p && isspace((unsigned char)*p))
        p++;
    return p;
}
// Boolean parser.
static bool json_parse_bool(const char *buf, const char *key, bool *out)
{
    const char *p = json_find_key_value(buf, key);
    if (!p)
        return false;
    if (strncmp(p, "true", 4) == 0)
    {
        *out = true;
        return true;
    }
    if (strncmp(p, "false", 5) == 0)
    {
        *out = false;
        return true;
    }
    // also accept 1/0
    if (*p == '1')
    {
        *out = true;
        return true;
    }
    if (*p == '0')
    {
        *out = false;
        return true;
    }
    return false;
}

// Float parser (temperatur er en float gg nigger)
static bool json_parse_float(const char *buf, const char *key, float *out)
{
    const char *p = json_find_key_value(buf, key);
    if (!p)
        return false;
    char *endptr;
    double v = strtod(p, &endptr);
    if (p == endptr)
        return false;
    *out = (float)v;
    return true;
}

// ================= CHECK WEATHER (GET) =================
// host: hostname (e.g. iot-servofunctionapp-...swedencentral-01.azurewebsites.net)
// path: path part (e.g. /api/ServoFunctionContainerWebApp)
// retries: number of attempts
// timeout_ms: timeout per attempt in milliseconds
// returns true on success (and fills out parameters)
bool network_check_weather(bool *out_rainNextHour,
                           float *out_temperature,
                           uint8_t retries,
                           uint32_t timeout_ms)
{
    if (!out_rainNextHour || !out_temperature)
        return false;

    char request[512];
    sprintf(request,
            "GET /api/ServoFunctionContainerWebApp HTTP/1.1\r\n"
            "Host: iot-servofunctionapp-htbgfgatcpf9asch.swedencentral-01.azurewebsites.net\r\n"
            "Connection: close\r\n"
            "\r\n");

    for (uint8_t attempt = 1; attempt <= retries; attempt++)
    {
        tcp_received = false;
        memset(tcp_rx_buffer, 0, sizeof(tcp_rx_buffer));

        // Create TCP connection on port 80 (plain TCP)
        if (wifi_command_create_TCP_connection("51.12.31.5", 80, tcp_callback, tcp_rx_buffer) != WIFI_OK)
        {
            printf("[NETWORK] TCP connect attempt %d failed\n", attempt);
            _delay_ms(500);
            continue;
        }

        _delay_ms(200); // let module settle

        // Send request
        if (wifi_command_TCP_transmit((uint8_t *)request, strlen(request)) != WIFI_OK)
        {
            printf("[NETWORK] TCP transmit attempt %d failed\n", attempt);
            wifi_command_close_TCP_connection();
            _delay_ms(200);
            continue;
        }

        // Wait for response up to timeout_ms
        uint32_t waited = 0;
        const uint32_t poll_ms = 100;
        
        while (!tcp_received && waited < timeout_ms)
        {
            _delay_ms(poll_ms);
            waited += poll_ms;
        }

        if (!tcp_received)
        {
            printf("[NETWORK] No response on attempt %d (timeout %lu ms)\n", attempt, (unsigned long)timeout_ms);
            wifi_command_close_TCP_connection();
            _delay_ms(200);
            continue;
        }

        // Response received, extract body
        const char *body = strstr(tcp_rx_buffer, "\r\n\r\n");
        body = body ? body + 4 : NULL;
        if (!body)
        {
            printf("[NETWORK] No HTTP body found on attempt %d\n", attempt);
            wifi_command_close_TCP_connection();
            _delay_ms(200);
            continue;
        }

        // DEBUG: Se hvad der kommer tilbage
        printf("[NETWORK] Raw buffer: %s\n", tcp_rx_buffer);
        printf("[NETWORK] Body content: %s\n", body);
        // Parse expected JSON fields
        bool parsed_ok = true;
        if (!json_parse_bool(body, "\"rainNextHour\"", out_rainNextHour))
            parsed_ok = false;
        if (!json_parse_float(body, "\"temperature\"", out_temperature))
            *out_temperature = 0.0f; // not fatal

        wifi_command_close_TCP_connection();

        if (parsed_ok)
        {
            return true;
        }
        else
        {
            printf("[NETWORK] JSON parse failed on attempt %d\n", attempt);
            _delay_ms(200);
            continue;
        }
    }

    return false;
}