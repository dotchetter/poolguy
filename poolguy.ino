/*
    PoolGuy project; main firmware file

    Hardware configuration:
        MCU:                    Arduino MKR 1010 WiFi (SAMD21)
        TEMP SENSOR:            DSB18B20 ONEWIRE    (Connected to D8)
*/

#include "defines.h"
#include "DS18B20.h"
#include <StateMachine.h>
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

/* States enums */
enum class States
{
    IDLE,
    TRANSMIT_TELEMETRY,
    WIFI_CONNECT,
    DEEP_SLEEP
};

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

    stateMachine.setChainedState(States::TRANSMIT_DATA, States::DEEP_SLEEP);
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


void transmit_data()
{    
    char serializedJsonOutput[128];
    byte isCharging = 0;
    float voltage = read_battery_voltage();
    int batterylevel = read_batterylevel();
    float waterTemperature = tempSensor.GetTemperature('C');
    float humidity = dhtSensor.readHumidity();
    float airTemperature = dhtSensor.readTemperature();


    if (voltage > BATT_MAX_V)
        isCharging = 1;
  
    // Assemble the body of the POST message:
    StaticJsonDocument<128> jsonDocument;
    jsonDocument["water_temp"] = waterTemperature;
    jsonDocument["battery_level"] = batterylevel;
    jsonDocument["is_charging"] = isCharging;
    jsonDocument["air_temp"] = airTemperature;
    jsonDocument["air_humidity"] = humidity;

    // Serialize the JSON object to char array
    serializeJson(jsonDocument, serializedJsonOutput);

    #ifdef DEVMODE
        Serial.println("making POST request");
        Serial.println(serializedJsonOutput);
    #endif

    // send the POST request
    client.post(SUB_PATH, CONTENT_TYPE, serializedJsonOutput);
        
    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    
    #ifdef DEVMODE
        Serial.print("Status code: ");
        Serial.println(statusCode);
        Serial.print("Response: ");
        Serial.println(client.responseBody());
    #endif

    flash(waterTemperature, 180);

    stateMachine.release();
}


int flash(int times, int ms_delay)
{
    int state = 0;
    for (int i = 0; i < times * 2; i++)
    {
        digitalWrite(STATUS_LED_PIN, state);
        delay(ms_delay);
        state = !state;
    }
    digitalWrite(STATUS_LED_PIN, 0);
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