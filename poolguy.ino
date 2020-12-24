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
void sleep();

/* Arduino IoT Cloud properties */
float temp;

/* State enums */
enum class State
{
    IDLE,
    TRANSMIT_TELEMETRY,
    SLEEP
};

/* Heap allocated globally accessible instances (Singletons) */
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
DS18B20 *tempSensor = new DS18B20(TEMP_PORT_GROUP, PORT_PA16);
StateMachine<State> *stateMachine = new StateMachine<State>(State::IDLE, idle);


void setup()
{   
    /* Configure StateMachine instance with states and chains */
    stateMachine->addState(State::TRANSMIT_TELEMETRY, transmit_telemetry);
    stateMachine->addState(State::SLEEP, sleep);

    /* Configure integration with Arduino IoT Cloud */
    ArduinoCloud.setThingId(THING_ID);
    ArduinoCloud.addProperty(temp, READWRITE, 5 * SECONDS);
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
    
    /* Set the WiFi module in low power mode */
    WiFi.lowPowerMode();
}


void transmit_telemetry()
{
    temp = tempSensor->GetTemperature('C');
    ArduinoCloud.update();
    stateMachine->release();
}


void wifi_connect()
{
    static uint64_t last_led_update;

    if  (millis() - last_led_update > 2000)
    {
        Serial.println("Connecting");
        
        for (int i = 0; i < 12; i++)
        {
            digitalWrite(STATUS_LED_PIN, flash());
            delay(200);
        }

        WiFi.begin(WIFI_SSID, WIFI_PASS);
        last_led_update = millis();            
    }
}


void idle()
{
    static uint64_t last_sleep;
    
    if (millis() - last_sleep > RUN_TIME)
    {
        last_sleep = millis();
        stateMachine->transitionTo(State::SLEEP);
    }
    else
    {
        stateMachine->transitionTo(State::TRANSMIT_TELEMETRY);
        stateMachine.transitionTo(States::WIFI_CONNECT);
    }
}


void loop() 
{
    stateMachine->next()();
}