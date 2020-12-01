/*
    PoolGuy project; main firmware file

    Hardware configuration:
        MCU:                    Arduino MKR 1010 WiFi (SAMD21)
        PH SENSOR:              GROVE E201C-BLUE    (Connected to D8)
        TEMP SENSOR:            DSB18B20 ONEWIRE    (Connected to A1)
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
DS18B20 *tempSensor = new DS18B20(TEMP_PORT_GROUP, PORT_PA16);
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
StateMachine<State> *stateMachine = new StateMachine<State>(State::IDLE, idle);


void tempChanged()
{
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" C");
}

    /* Configure StateMachine instance with states and chains */
    stateMachine->addState(State::TRANSMIT_TELEMETRY, transmit_telemetry);
    stateMachine->addState(State::WIFI_ERROR, wifi_error);

void setup()
{
    Serial.begin(112500);
    ArduinoCloud.setThingId(THING_ID);
    ArduinoCloud.addProperty(temp, READWRITE, 60 * SECONDS, tempChanged);
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);

    setDebugMessageLevel(2);
    ArduinoCloud.printDebugInfo();
    stateMachine->transitionTo(State::TRANSMIT_TELEMETRY);
}


uint32_t last_iter;
void loop() 
{
    temp = tempSensor->GetTemperature('C');
    Serial.println(temp);
    last_iter = millis();
    ArduinoCloud.update();
    LowPower.sleep(STDBY_TIME_MS);
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