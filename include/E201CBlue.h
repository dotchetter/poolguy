/*
 * File: E201CBlue.h
 * Author: Simon Olofsson
 * Program: Source file for driver module
 * Processor: SAMD21G18A @ 48MHz, 3.3v
 * Compiler: ARM-GCC 
 * MCU: Arduino MKR 1010 WiFi
 * Program Description: This file contains source code
                        for the driver for the Grove E201C-Blue
                        PH sensor. It provides a user interface
                        for developers that work with this device.

 * Sensor description:  Operating voltage       3.3V/5V
                        Range                   0-14PH
                        Resolution              ±0.15PH（STP）
                        Response time           ＜1min
                        Probe Interface         BNC
                        Measure temperature     0-60℃
                        Internal resistance     ≤250MΩ（25℃）
                        Alkali error            0.2PH（1mol/L）Na+，PH14)（25℃）
 *
 * Written on 2020-11-10, Simon Olofsson
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef E201CBLUE_H_
#define E201CBLUE_H_

#include "sam.h"
#include <stdint.h>


class E201C
/*
* Instantiate this class for each E201C sensor
* attached to the device. 
*
* FIELD: deviationOffset: deviationOffset is calculated
*                         to account for deviations in 
*                         the linear characteristics of
*                         PH detection.
*
* FIELD: referenceVoltage: The voltage the sensor will be
*                          supplied with.
*
* FIELD: sensorMaxValue:  The maximum value (min is 0 by 
*                         default) - that the sensor will 
*                         report to the device. This is 
*                         subject to the ADC conversion on
*                         the device. This value is used in 
*                         PH evaluation,so it is important 
*                         that it's accurate. For most 
*                         implementations, 1023 (10 bit ADC) 
*                         is used.
*
* FIELD: phRef 1 & 2:     The reference PH of the calibration
*                         buffer fluids.
*
* FIELD: V1 & V2 Value   Represents the volume of buffer 1 and 2
*                        to get accurate dilutions
*
* FIELD: kValue:         The K value is a result of a subtraction
*                        between the PH reference values 
*                        (phref2-phref1) divided by V1Value 
*                        and V2Value
* 
* FIELD: channel:        The registry channel associated with the 
*                        GPIO pin on the board which the sensor 
*                        SIG pin is connected to. Refer to the datasheet
*                        for the Arduino MKR 1010 for this info.
*
* FIELD measurements:    Array concisting of 50 elements available
*                        for measurements with the sensor. 
*                        This array is later averaged which is
*                        the measured PH value.
*
* FIELD phValue:         Represents the recent most PH reading.
*
* FIELD recentMostReadingAvg: Represents the recent most measurement average
*                             used in calculating voltage and PH.
*/
{
private:
    float deviationOffset;
    float referenceVoltage;
    float phRef1;
    float phRef2;
    float V1Value = 1.93;
    float V2Value = 1.66;
    float kValue;
    float phValue;
    float recentMostReadingAvg = 0.0;
    uint8_t channel;
    uint32_t numSamples = 50;
    uint32_t sensorMaxValue;
    void Read();
    
public:
    E201C(float referenceVoltage, uint32_t sensorMaxValue, 
          float phRef1, float phRef2, uint8_t channel);

    /* Getters */
    const float GetDeviationOffset() const;
    const float GetReferenceVolatage() const;
    const uint32_t GetSensorMaxValue() const;
    const float GetPhReferenceValue(int num) const;
    const float GetVolumeReferenceValue(int num) const;
    const float GetKValue() const;
    const float GetSensorVoltage();
    const float GetSensorPH();
    const uint8_t GetChannel() const;
    const uint32_t GetNumSamples() const;

    /* Setters */
    void SetDeviationOffset(float val);
    void SetReferenceVolatage(float val);
    void SetSensorMaxValue(uint32_t val);
    void SetPhReferenceValue(int num, float val);
    void SetVolumeReferenceValue(int num, float val);
    void SetChannel(uint8_t channel);
    void SetNumSamples(uint32_t val);
};

#endif // E201C-BLUE_DRV_H_
#ifdef __cplusplus
}
#endif // __cplusplus}