#include "wifi.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/delay.h>
#include "uart.h"

#define WIFI_DATABUFFERSIZE 128

static uint8_t wifi_dataBuffer[WIFI_DATABUFFERSIZE];
static uint16_t wifi_dataBufferIndex;
static uint32_t wifi_baudrate;

static void (*_callback)(uint8_t byte);

// TCP receive
static void wifi_TCP_callback(uint8_t byte);

// ================= UART CALLBACK =================
static void wifi_callback(uint8_t received_byte)
{
    if (_callback != NULL)
        _callback(received_byte);
}

// ================= COMMAND CALLBACK =================
static void wifi_command_callback(uint8_t received_byte)
{
    if (wifi_dataBufferIndex < WIFI_DATABUFFERSIZE - 1)
    {
        wifi_dataBuffer[wifi_dataBufferIndex++] = received_byte;
        wifi_dataBuffer[wifi_dataBufferIndex] = '\0';
    }
}

// ================= INIT =================
void wifi_init()
{
    wifi_baudrate = 115200;
    uart_init(UART2_ID, wifi_baudrate, wifi_callback, 0);
}

// ================= BUFFER CLEAR =================
static void wifi_clear_databuffer_and_index()
{
    memset(wifi_dataBuffer, 0, WIFI_DATABUFFERSIZE);
    wifi_dataBufferIndex = 0;
}

// ================= GENERIC COMMAND =================
WIFI_ERROR_MESSAGE_t wifi_command(const char *str, uint16_t timeout_s)
{
    void *old_callback = _callback;
    _callback = wifi_command_callback;

    wifi_clear_databuffer_and_index();

    uart_send_string_blocking(UART2_ID, str);
    uart_send_string_blocking(UART2_ID, "\r\n");

    for (uint16_t i = 0; i < timeout_s * 100; i++)
    {
        _delay_ms(10);

        if (strstr((char *)wifi_dataBuffer, "OK") != NULL)
            break;
    }

    WIFI_ERROR_MESSAGE_t error;

    if (wifi_dataBufferIndex == 0)
        error = WIFI_ERROR_NOT_RECEIVING;
    else if (strstr((char *)wifi_dataBuffer, "OK") != NULL)
        error = WIFI_OK;
    else if (strstr((char *)wifi_dataBuffer, "ERROR") != NULL){
        error = WIFI_ERROR_RECEIVED_ERROR;
    }
    else if (strstr((char *)wifi_dataBuffer, "FAIL") != NULL)
        error = WIFI_FAIL;
    else
        error = WIFI_ERROR_RECEIVING_GARBAGE;

    wifi_clear_databuffer_and_index();
    _callback = old_callback;

    return error;
}

// ================= BASIC COMMANDS =================
WIFI_ERROR_MESSAGE_t wifi_command_AT()
{
    return wifi_command("AT", 1);
}

WIFI_ERROR_MESSAGE_t wifi_command_disable_echo()
{
    return wifi_command("ATE0", 1);
}

WIFI_ERROR_MESSAGE_t wifi_command_set_mode_to_1()
{
    return wifi_command("AT+CWMODE=1", 1);
}

WIFI_ERROR_MESSAGE_t wifi_command_set_to_single_Connection()
{
    return wifi_command("AT+CIPMUX=0", 1);
}

WIFI_ERROR_MESSAGE_t wifi_command_join_AP(char *ssid, char *password)
{
    char cmd[256];

    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    return wifi_command(cmd, 20);
}

WIFI_ERROR_MESSAGE_t wifi_command_close_TCP_connection()
{
    return wifi_command("AT+CIPCLOSE", 5);
}

// ================= TCP RECEIVE =================
#define IPD_PREFIX "+IPD,"

WIFI_TCP_Callback_t callback_when_message_received_static;
char *received_message_buffer_static_pointer;

static void wifi_TCP_callback(uint8_t byte)
{
    static enum { IDLE,
                  LENGTH,
                  DATA } state = IDLE;
    static int length = 0, index = 0;

    static char prefix[5];
    static uint8_t prefix_index = 0;

    switch (state)
    {
    case IDLE:
        if (byte == '+')
        {
            prefix_index = 0;
            prefix[prefix_index++] = byte;
            state = LENGTH;
        }
        break;

    case LENGTH:
        if (prefix_index < 5)
        {
            prefix[prefix_index++] = byte;

            if (prefix_index == 5)
            {
                if (strncmp(prefix, IPD_PREFIX, 5) != 0)
                {
                    state = IDLE;
                }
                else
                {
                    length = 0;
                }
            }
        }
        else if (byte >= '0' && byte <= '9')
        {
            length = length * 10 + (byte - '0');
        }
        else if (byte == ':')
        {
            index = 0;
            state = DATA;
        }
        break;

    case DATA:
        if (index < length)
        {
            received_message_buffer_static_pointer[index++] = byte;
        }

        if (index == length)
        {
            received_message_buffer_static_pointer[index] = '\0';

            callback_when_message_received_static();

            state = IDLE;
            length = 0;
            //index = 0; ikke nulstil index gg nigger
        }
        break;
    }
}

// ================= CREATE TCP =================
WIFI_ERROR_MESSAGE_t wifi_command_create_TCP_connection(
    const char *IP,
    uint16_t port,
    WIFI_TCP_Callback_t callback,
    char *rx_buffer)
{
    received_message_buffer_static_pointer = rx_buffer;
    callback_when_message_received_static = callback;

    char cmd[256];
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%u", IP, port);

    
    WIFI_ERROR_MESSAGE_t err = wifi_command(cmd, 10);

    if (err != WIFI_OK)
        return err;

    _callback = wifi_TCP_callback;

    return WIFI_OK;
}
// test for callback fejl i forbindelse med respons
static void (*tx_forward_callback)(uint8_t byte) = NULL;

static void wifi_tx_combined_callback(uint8_t byte)
{
    // Behold command parsing for '>' og 'SEND OK'
    wifi_command_callback(byte);

    // Forward samtidig bytes til TCP parser (+IPD handler)
    if (tx_forward_callback != NULL)
    {
        tx_forward_callback(byte);
    }
}

// ================= SEND TCP =================
WIFI_ERROR_MESSAGE_t wifi_command_TCP_transmit(uint8_t *data, uint16_t length)
{
    char cmd[32];

    // Gem den nuværende callback så vi kan gendanne den senere
    void (*old_callback)(uint8_t) = _callback;

    // Skift til command-callback for at fange '>' og 'SEND OK'
    tx_forward_callback = old_callback;
    _callback = wifi_tx_combined_callback;
    wifi_clear_databuffer_and_index();

    sprintf(cmd, "AT+CIPSEND=%u", length);
    uart_send_string_blocking(UART2_ID, cmd);
    uart_send_string_blocking(UART2_ID, "\r\n");

    // vent på >
    uint16_t timeout = 0;
    while (timeout < 3000)
    {
        _delay_ms(10);

        if (strchr((char *)wifi_dataBuffer, '>') != NULL)
            break;

        timeout++;
    }

    if (timeout >= 300)
    {
        // Gendan tidligere callback før vi returnerer
        _callback = old_callback;
        tx_forward_callback = NULL;
        return WIFI_FAIL;
    }

    // send data
    uart_write_bytes(UART2_ID, data, length);

    // vent på SEND OK
    timeout = 0;
    while (timeout < 3000)
    {
        _delay_ms(10);

        if (strstr((char *)wifi_dataBuffer, "SEND OK") != NULL)
        {
            break;
        }

        timeout++;
    }

    // Ryd buffer og gendan TCP-callback, så indkommende +IPD kan behandles
    _callback = old_callback;
    tx_forward_callback = NULL;

    return WIFI_OK;
}