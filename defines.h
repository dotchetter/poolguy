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

#define REF_VOLTAGE          3.3f
#define ADC_MAX_VAL_REF      1024ul
#define PH_REF_1             4.0f
#define PH_REF_2             9.18
#define PH_SENSOR_PIN        A1
#define PH_SENSOR_CHANNEL    10u

#define TEMP_PORT_GROUP      0
#define TEMP_PINMASK         PORT_PA16
#define WIFI_RECONNECT_WAIT  3000

#define WIFI_SSID            ""
#define WIFI_PASS            ""
#define DEVICE_ID            ""
#define STDBY_TIME_MS        2000

#endif // DEFINITIONS_H_
