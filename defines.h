#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

/*
 * File: definitions.h
 * Author: Simon Olofsson
 * Program: 'PoolGuy' project firmware
 * Processor: SAMD21G18A @ 48MHz, 3.3v
 * Compiler: ARM-GCC 
 * MCU: Arduino MKR 1010 WiFi
 * Program Description: This file contains definitions
                        used in the firmware.
 *
 * Written on 2020-11-26, Simon Olofsson
 */

#define TEMP_PORT_GROUP      0
#define TEMP_PINMASK         PORT_PA16
#define STATUS_LED_PIN       5
#define MAIN_BUTTON_PIN      6

#define SECOND               1000UL
#define MINUTE               60 * SECOND

#define WIFI_SSID            ""
#define WIFI_PASS            ""
#define DEVICE_ID            ""
#define INTERVAL             10
#define UPDATE_CYCLES        F_CPU / 1000000UL
#define BATT_DELAY           1 * MINUTE
#define BATT_MAX_V           3.950
#define BATT_MIN_V           3.160

#define DEBUG                1

#endif // DEFINITIONS_H_
