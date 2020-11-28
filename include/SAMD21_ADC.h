/*
 * File:   SAMD21_ADC.cpp
 * Author: Simon Olofsson
 * Program: ADC file to initialize the Analog-digital converter
 *          for the SAMD21 MCU.
 * Compiler: ARM-GCC 
 * Program Version 1.0
 * Program Description: This file contains source code
                        for the driver for the E201C-Blue
                        PH sensor. It provides a user interface
                        for developers that work with this device.
 *
 * Written on 2020-11-10, Simon Olofsson
 */


void init_adc()
/*
* Initialize the analog-digital converter
*/
{
    /* Set the ADC as 10 bit resolution register (0 -> 1023) */
    ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_10BIT_Val;

    /* Do not run ADC in deep sleep but keep the value cached */
    ADC->CTRLA.bit.RUNSTDBY = 0x0;

    /* The accuracy of the gain stage can be increased 
       by enabling the reference buffer offset compensation.
       This will decrease the input impedance and thus 
       increase the start-up time of the reference.
       This enables this feature. */
    ADC->REFCTRL.bit.REFCOMP = 0x1;

    /* Disable differential mode */
    ADC->CTRLB.bit.DIFFMODE = 0x0;
    
    /* Enable Free running mode (No need to start conversion manually) */
    ADC->CTRLB.bit.FREERUN = 0x1;
    
    /* Configure result value as right adjusted */
    ADC->CTRLB.bit.LEFTADJ = 0x0;

    /* Disable the digital result correction */
    ADC->CTRLB.bit.CORREN = 0x0;     

    /* Enable ADC */
    ADC->CTRLA.reg = ADC_CTRLA_ENABLE;
}