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
#include <DHT.h>

/* Constants */
const char THING_ID[] = DEVICE_ID;
const char SSID[]     = WIFI_SSID;
const char PASS[]     = WIFI_PASS;

/* Method declarations */
void idle();
void transmit_telemetry();
void wifi_connect();
void deep_sleep();

/* Arduino IoT Cloud properties */
float temp;
int batterylevel;

/* States enums */
enum class States
{
    IDLE,
    TRANSMIT_TELEMETRY,
    WIFI_CONNECT,
    DEEP_SLEEP
};

/* Heap allocated globally accessible instances (Singletons) */
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
DHT dhtSensor = DHT(DHT_SENSOR_PIN, DHT11);
DS18B20 tempSensor = DS18B20(TEMP_PORT_GROUP, PORT_PA16);
StateMachine<States> stateMachine = StateMachine<States>(States::IDLE, idle);

void setup()
{
    Serial.begin(38400);
    dhtSensor.begin();
    WiFi.lowPowerMode();

    /* Configure peripherals */
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(MAIN_BUTTON_PIN, INPUT);
    
    /* Configure StateMachine instance with states and chains */
    stateMachine.addState(States::TRANSMIT_TELEMETRY, transmit_telemetry);
    stateMachine.addState(States::WIFI_CONNECT, wifi_connect);
    stateMachine.addState(States::DEEP_SLEEP, deep_sleep);

    stateMachine.setChainedState(States::TRANSMIT_TELEMETRY, States::DEEP_SLEEP);

    /* Configure integration with Arduino IoT Cloud */
    ArduinoCloud.setThingId(THING_ID);
    ArduinoCloud.addProperty(temp, READWRITE);
    ArduinoCloud.addProperty(batterylevel, READWRITE);
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
}


int read_batterylevel()
{
    float voltage = 0.0;
    float output = 0.0;
    uint16_t battery_level;

    /* Read the value on ADC_BATTERY ( 0 - 1023 ) */
    battery_level = analogRead(ADC_BATTERY);
    
    /* Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 4.3V): */
    float readvoltage = (battery_level * (4.3 / 1023.0)) * 0.969543;
  
    /* round value by two precision */
    voltage = roundf(readvoltage * 100) / 100;
    output = round((voltage - BATT_MIN_V) / (BATT_MAX_V - BATT_MIN_V) * 100);

    if (output >= 0 && output < 100)
    {
        return round(output);   
    }      
    else if (output < 0)
    {
        return 1;
    }
    return 100;
}


void deep_sleep()
{
    LowPower.deepSleep(INTERVAL);
    stateMachine.release();
}


void transmit_telemetry()
{    
    batterylevel = read_batterylevel();
    temp = tempSensor.GetTemperature('C');
    
    #if DEVMODE
        Serial.print("Transmitting temperature: ");Serial.print(temp);Serial.println(" C");
        Serial.print("Transmitting battery: "); Serial.print(batterylevel);Serial.println("%");
    float waterTemperature = tempSensor.GetTemperature('C');
    float humidity = dhtSensor.readHumidity();
    #endif

    for (int i = 0; i < 12; i++)
    {
        digitalWrite(STATUS_LED_PIN, flash());
        delay(45);
    }
    
    for (int i = 0; i < UPDATE_CYCLES; i++)
    {
        ArduinoCloud.update();
    }    
    
    stateMachine.release();
}


int flash()
{
    static uint8_t last_value = 0;
    
    last_value = !last_value;
    return last_value;
}


void wifi_connect()
{
    #if DEVMODE
        Serial.print("Connecting to WiFI... ");
    #endif

    for (int i = 0; i < 12; i++)
    {
        digitalWrite(STATUS_LED_PIN, flash());
        delay(200);
    }

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    #if DEVMODE
        WiFi.status() == WL_CONNECTED ? Serial.println("success") : Serial.println("failed");
    #endif
}


void idle()
{    
    if (WiFi.status() == WL_CONNECTED)
    {     
        stateMachine.transitionTo(States::TRANSMIT_TELEMETRY);
    }    
    else
    {
        stateMachine.transitionTo(States::WIFI_CONNECT);
    }
}


void loop() 
{
    stateMachine.next()();
}