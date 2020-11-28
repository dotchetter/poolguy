/*
 * File: DS18B20.h
 * Author: Simon Olofsson
 * Program: Source file for driver module
 * Compiler: ARM-GCC 
 * Program Version 1.0
 * Program Description: This file contains source code
                        for the driver for the DSB18B20
                        onewire sensor module. It is not
                        designed for a one wire with several
                        devices to the same line, but takes
                        the pin it is connected to as parameter
                        and will only adress one of the devices.
 *
 * Written on 2020-11-13, Simon Olofsson
 */

#include "include/port.h"
#include "Arduino.h"
#include "math.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DS12B20_H_
#define DS12B20_H_

/*

The SAMD21 MCU has a different port layout than
many users and developers are used to, coming 
from the ATMEGA328P or alike. To use this driver,
you need to provide the port group and the pinmask
for the pin you selected to plug in the sensor to.

To make this easier for you, we have provided you
with this lookup table to find this information.

Buy us a coffee? https://www.buymeacoffee.com/dotchetter

    **********************************
    * PHYSICAL-PIN   MCU       GROUP *
    **********************************
        AREF        PA03        0
        A0/D15      PA02        0
        A1/D16      PB02        1
        A2/D17      PB03        1
        A3/D18      PA04        0
        A4/D19      PA05        0
        A5/D20      PA06        0
          D0        PA22        0
          D1        PA23        0
          D2        PA10        0
          D3        PA11        0
          D4        PB10        1
          D5        PB11        1
          D14       PB22        1
          D13       PB23        1
          D12       PA09        0
          D11       PA08        0
          D10       PA19        0
          D9        PA17        0
          D8        PA16        0
          D7        PA21        0
          D6        PA20        0
*/


/* 
* DSB18B20 Commands in hex form. See datasheet pg 11
* https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
*/

#define READ_ROM_CMD 0x33
#define MATCH_ROM_CMD 0x55
#define SKIP_ROM_CMD 0xCC
#define ALARM_SRCH_CMD 0xEC
#define CPY_SCRTC_PD_CMD 0x48
#define RECALL_E2_CMD 0xB8
#define SRCH_ROM_CMD 0xF0
#define CONVERT_CMD 0x44
#define WRITE_SCRTC_PD_CMD 0x4E
#define READ_SCRTC_PD_CMD 0xBE
#define READ_PW_SUPPL_CMD 0xB4
#define NUM_SCRATCHPAD_BYTES 0x09
#define BITS_IN_BYTE 8

/* 
* Data processing is handled on-chip of the sensor.
* According to the datasheet, this time needs to be
* accounted for. These defined describe these periods.
*/

#define TIME_SLOT_US 60
#define RECOVERY_US 1
#define WRITE_1_LOW_US 60
#define WRITE_0_LOW_US 1
#define RESET_US 480
#define PRECEDENCE_DETECT_HIGH_US 15
#define PRECEDENCE_DETECT_LOW_US 60
#define CONVERSION_TIME_US 750

/* Bitmasks */
#define READING_MSB_MASK 0x07

/* Constants */
#define FRACTION_LSB 4
#define FRACTION_CALC_BASE -2.0f
#define FRACTION_CALC_EXP -4.0f


class DS18B20
{
/*
* Instantiate this class for each E201C sensor
* attached to the device. 
*
* FIELD: pinMask:          pinMask refers to the port (UL)
*                          that the microcontroller associates
*                          with the given pin the sensor is connected
*                          to. To find this, look at the table above.
*
* METHOD: _us_to_cycles:   Convert microseconds to cpu cycles
*
* METHOD: SetAsInput:      Configure the pin where the sensor is
*                          plugged in as input - a part of the
*                          means of communication over onewire.
*
* METHOD: SetAsOutput:     Configure the pin where the sensor is
*                          plugged in as output - a part of the
*                          means of communication over onewire.
*
* METHOD: BusWrite:        Write a 1 (HIGH) or 0 (LOW)to the bus 
*                          where the device is connected.
*
* METHOD BusRead:          Read from the bus where the device
*                          is connected.
*
* METHOD: InitCommand:     Selecting the pin as output and writing a LOW
*                          will let the sensor know there is an upcoming
*                          transition of command(s).
*
* METHOD: SuspendMicroSeconds: Allows the caller to return n cycles to sleep for
*                              desired amount of microseconds.
*
* METHOD: SendResetCommand:  Resets the sensor device by sensing the initial
*                            command, then switching on the line as input
*
* METHOD: SendByteCommand:  Send a command to the sensor. 
*                           The commands for the DS18B20 are bytes
*                           where each bit is part of an instruction.
*                           This method sends one bit at a time, where
*                           inbetween, the application must be suspended for
*                           a defined amount of microseconds for the sensor
*                           to process the command.
*
* METHOD: ReadScratchPad:   Read one byte of data from the sensor.
*                           The size of a byte is iterated over, and
*                           ultimately concatenates a byte from the 
*                           sensor received over one wire.
*/
private:
    uint32_t pinMask;
    uint8_t portGroup;
    uint64_t _us_to_cycles(uint32_t us);
    void SetAsInput();
    void SetAsOutput();
    void BusWrite(uint8_t mode);
    uint32_t BusRead();
    void InitCommand();
    void SuspendMicroSeconds(uint32_t microSeconds);
    void SendResetCommand();
    void SendByteCommand(uint8_t command);
    uint8_t ReadScratchPad();

public:
    DS18B20(uint8_t portGroup, uint32_t pinMask);
    ~DS18B20();
    float GetTemperature(const char unit);
};


#endif // DS12B20_H_
#ifdef __cplusplus
}
#endif // __cplusplus}