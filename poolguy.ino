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
void wifi_connect();

/* Arduino IoT Cloud properties */
float temp;
int batterylevel;
int interval = INTERVAL;

/* States enums */
enum class States
{
    IDLE,
    TRANSMIT_TELEMETRY,
    WIFI_CONNECT
};

/* Heap allocated globally accessible instances (Singletons) */
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
DS18B20 tempSensor = DS18B20(TEMP_PORT_GROUP, PORT_PA16);
StateMachine<States> stateMachine = StateMachine<States>(States::IDLE, idle);


void setup()
{
    Serial.begin(38400);
    //DBG_INFO(F("*"));
    digitalWrite(STATUS_LED_PIN, flash());

    /* Configure peripherals */
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(MAIN_BUTTON_PIN, INPUT);

    /* Configure StateMachine instance with states and chains */
    stateMachine.addState(States::TRANSMIT_TELEMETRY, transmit_telemetry);
    stateMachine.addState(States::WIFI_CONNECT, wifi_connect);

    /* Configure integration with Arduino IoT Cloud */
    ArduinoCloud.setThingId(THING_ID);
    ArduinoCloud.addProperty(temp, READWRITE, ON_CHANGE);
    ArduinoCloud.addProperty(batterylevel, READWRITE, ON_CHANGE);
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
}


int read_batterylevel()
{
    uint32_t sum = 0;
    float voltage = 0.0;
    float output = 0.0;
    const float battery_max = 3.99;
    const float battery_min = 3.0;

    /* Read the value on ADC_BATTERY ( 0 - 1023 ) */
    int battery_level = analogRead(ADC_BATTERY);
    
    /* Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 4.3V): */
    float readvoltage = (battery_level * (4.3 / 1023.0)) * 0.969543;
  
    /* round value by two precision */
    voltage = roundf(readvoltage * 100) / 100;
    output = round((voltage - battery_min) / (battery_max - battery_min) * 100);
    
    if (output >= 0 && output < 100)
    {
        return output;   
    }      
    else if (output < 0)
    {
        return -1.0f;
    }

    return 100.0f;
}


void transmit_telemetry()
{
    static uint64_t last_batt_update;

    /* Introduce artificial cusioning of battery percentage */ 
    if (millis() - last_batt_update > BATT_DELAY)
    {
        batterylevel = read_batterylevel();
        last_batt_update = millis();
    }

    temp = tempSensor.GetTemperature('C');
    
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
    static uint64_t last_run;
    
    if (WiFi.status() == WL_CONNECTED)
    {
        digitalWrite(STATUS_LED_PIN, HIGH);
     
        if (millis() - last_run > interval * SECOND)
        {
            for (int i = 0; i < 12; i++)
            {
                digitalWrite(STATUS_LED_PIN, flash());
                delay(75);
            }
     
            Serial.println("Transmitting");
            stateMachine.transitionTo(States::TRANSMIT_TELEMETRY);
            last_run = millis();        
        }
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