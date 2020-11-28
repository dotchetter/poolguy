#include "E201CBlue.h"

/*
 * File:   E201CBlue.cpp
 * Author: Simon Olofsson
 * Program: Source file for driver module
 * Compiler: ARM-GCC 
 * Program Version 1.0
 * Program Description: This file contains source code
                        for the driver for the E201C-Blue
                        PH sensor. It provides a user interface
                        for developers that work with this device.
 *
 * Written on 2020-11-10, Simon Olofsson
 */


E201C::E201C(float referenceVoltage, uint32_t sensorMaxValue, 
             float phRef1, float phRef2, uint8_t channel)
{
    this->referenceVoltage = referenceVoltage;
    this->sensorMaxValue = sensorMaxValue;
    this->phRef1 = phRef1;
    this->phRef2 = phRef2;
    this->channel = channel;
    this->kValue = this->GetKValue();
    this->deviationOffset = this->GetDeviationOffset();
}


const float E201C::GetDeviationOffset() const
/* Returns the deviation offset based upon 
   datasheet formula:  Offset=[(PH2+PH1)-k*(V1+V2)]/2
*/
{
    return ((this->phRef1 + this->phRef2) - this->GetKValue() * (this->V1Value + this->V2Value)) / 2;
}

/* Getters */

const float E201C::GetReferenceVolatage() const
{
    return this->referenceVoltage;
}


const uint32_t E201C::GetSensorMaxValue() const
{
    return this->sensorMaxValue;
}


const float E201C::GetPhReferenceValue(int num) const
{
    switch(num)
    {
        case 1: return this->phRef1; break;
        case 2: return this->phRef2; break;
    }
}


const float E201C::GetVolumeReferenceValue(int num) const
{
    switch(num)
    {
        case 1: return this->V1Value; break;
        case 2: return this->V2Value; break;
    }
}


const float E201C::GetKValue() const
/*
* Returns the K value based on set parameter for the sensor.
*/
{
    return ((float)this->phRef2 - this->phRef1) / ((float)this->V2Value - this->V1Value);
}


const float E201C::GetSensorVoltage()
{
    this->Read();
    return (this->recentMostReadingAvg * this->referenceVoltage) / this->sensorMaxValue;
}


const float E201C::GetSensorPH()
{
    this->Read();
    return this->GetKValue() * this->GetSensorVoltage() + this->deviationOffset;
}


const uint8_t E201C::GetChannel() const
{
    return this->channel;
}


const uint32_t E201C::GetNumSamples() const
{
    return this->numSamples;
}

/* Setters */

void E201C::SetDeviationOffset(float val)
{
    this->deviationOffset = val;
}


void E201C::SetReferenceVolatage(float val)
{
    this->referenceVoltage = val;
}


void E201C::SetSensorMaxValue(uint32_t val)
{
    this->sensorMaxValue = val;
}


void E201C::SetPhReferenceValue(int num, float val)
{
    switch(num)
    {
        case 1: this->phRef1 = val; break;
        case 2: this->phRef2 = val; break;
    }
}


void E201C::SetVolumeReferenceValue(int num, float val)
{
    switch(num)
    {
        case 1: this->V1Value = val; break;
        case 2: this->V2Value = val; break;
    }
}


void E201C::SetChannel(uint8_t channel)
{
    this->channel = channel;
}


void E201C::SetNumSamples(uint32_t val)
{
    this->numSamples = val;
}

void E201C::Read()
/*
* Read the pin associated with 'this' instance
* by switching the INPUTCTRL bit on MUXPOS to
* the channel assigned to 'this'. 
*
* Updates the instance property 'recentMostReadingAvg'.
*/
{
    float sum = 0;
    
    /* Toggle 'this' channel as the current one to convert from analog */ 
    ADC->INPUTCTRL.bit.MUXPOS = this->channel;

    /* Start conversion and flush */
    ADC->SWTRIG.reg = ADC_SWTRIG_START | ADC_SWTRIG_FLUSH;

    /* Iterate and collect samples. Calculate the average. */
    for (int i = 0; i < this->numSamples; i++)
    {
        /* Retreive result from the ADC */
        sum += ADC->RESULT.reg;
        
        /* Await n cycles for the sensor to update */
        for (int j = 0; j < F_CPU/10000000; j++) {asm("nop");}
    }
    this->recentMostReadingAvg = (float)sum / this->numSamples;
}