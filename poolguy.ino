/*
    PoolGuy project; main firmware file

    Hardware configuration:
        MCU:                    Arduino MKR 1010 WiFi (SAMD21)
        TEMP SENSOR:            DSB18B20 ONEWIRE    (Connected to D8)
*/

#include "defines.h"
#include "DS18B20.h"
#include <StateMachine.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <ArduinoLowPower.h>

/* Constants */
const char THING_ID[] = DEVICE_ID;
const char SSID[]     = WIFI_SSID;
const char PASS[]     = WIFI_PASS;

/* Method declarations */
void idle();
void transmit_telemetry();
void wifi_error();

/* State enums */
enum class State
{
    IDLE,
    TRANSMIT_TELEMETRY,
    WIFI_ERROR
};

/* Arduino IoT Cloud properties */
float temp;

/* Heap allocated globally accessible instances (Singletons) */
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
DS18B20 *tempSensor = new DS18B20(TEMP_PORT_GROUP, PORT_PA16);
StateMachine<State> *stateMachine = new StateMachine<State>(State::IDLE, idle);


void setup()
{   
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 1);

    /* Configure StateMachine instance with states and chains */
    stateMachine->addState(State::TRANSMIT_TELEMETRY, transmit_telemetry);
    stateMachine->addState(State::WIFI_ERROR, wifi_error);

    /* Configure integration with Arduino IoT Cloud */
    ArduinoCloud.setThingId(THING_ID);
    ArduinoCloud.addProperty(temp, READWRITE, 60 * SECONDS);
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
    
    /* Set the WiFi module in low power mode */
    WiFi.lowPowerMode();
}


void idle()
{
    static uint64_t last_sleep;
    if (millis() - last_sleep > RUNTIME_MS)
    {
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
        last_sleep = millis();
        LowPower.sleep(SLEEP_MS);
        delay(1000);
        NVIC_SystemReset();
    }
    stateMachine->transitionTo(State::TRANSMIT_TELEMETRY);
}


void transmit_telemetry()
{
    temp = tempSensor->GetTemperature('C');
    ArduinoCloud.update();
    stateMachine->release();
}


void wifi_error()
{
    stateMachine->release();
}


void loop() 
{
    stateMachine->next()();
}