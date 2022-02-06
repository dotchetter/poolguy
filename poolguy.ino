/*
    PoolGuy project; main firmware file

    Hardware configuration:
        MCU:                    Arduino MKR 1010 WiFi (SAMD21)
        TEMP SENSOR:            DSB18B20 ONEWIRE    (Connected to D8)
*/


#include "defines.h"
#include "DS18B20.h"
#include <WiFiNINA.h>
#include <StateMachine.h>
#include <ArduinoLowPower.h>
#include <ArduinoHttpClient.h>
#include <DHT.h>
#include <ArduinoJson.h>


/* Constants */
const char THING_ID[] = DEVICE_ID;
const char SSID[]     = WIFI_SSID;
const char PASS[]     = WIFI_PASS;

/* Method declarations */
void idle();
void transmit_data();
void wifi_connect();
void deep_sleep();

/* States enums */
enum class States
{
    IDLE,
    TRANSMIT_DATA,
    WIFI_CONNECT,
    DEEP_SLEEP
};


/* Globally accessible singletons */
WiFiClient wifi;
HttpClient client = HttpClient(wifi, SERVER_ROOT, SERVER_PORT);
DHT dhtSensor = DHT(DHT_SENSOR_PIN, DHT11);
DS18B20 tempSensor = DS18B20(TEMP_PORT_GROUP, PORT_PA16);
StateMachine<States> stateMachine = StateMachine<States>(States::IDLE, idle);


void setup()
{
    Serial.begin(38400);
    tempSensor.begin();
    dhtSensor.begin();
    WiFi.lowPowerMode();

    /* Configure peripherals */
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(MAIN_BUTTON_PIN, INPUT);
    client.setTimeout(10 * SECOND);

    /* Set the main button as an interrupt, allowing it to stop sleep state */
    LowPower.attachInterruptWakeup(MAIN_BUTTON_PIN, transmit_data, CHANGE);

    /* Configure StateMachine instance with states and chains */
    stateMachine.addState(States::TRANSMIT_DATA, transmit_data);
    stateMachine.addState(States::WIFI_CONNECT, wifi_connect);
    stateMachine.addState(States::DEEP_SLEEP, deep_sleep);

    stateMachine.setChainedState(States::TRANSMIT_DATA, States::DEEP_SLEEP);
}


float read_battery_voltage()
{
    uint16_t battery_level;

    /* Read the value on ADC_BATTERY ( 0 - 1023 ) */
    battery_level = analogRead(ADC_BATTERY);
    float voltage = 0.0;
    
    /* Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 4.3V): */
    float readvoltage = (battery_level * (4.3 / 1023.0)) * 0.969543;
  
    /* round value by two precision */
    voltage = roundf(readvoltage * 100) / 100;
    
    return voltage;
}


int read_batterylevel()
{
    float voltage = read_battery_voltage();
    float output = round((voltage - BATT_MIN_V) / (BATT_MAX_V - BATT_MIN_V) * 100);

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
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
        
    #if DEVMODE
        WiFi.status() == WL_CONNECTED ? Serial.println("success") : Serial.println("failed");
    #endif

    flash(3, 1000);
}


void idle()
{    
    if (WiFi.status() == WL_CONNECTED)
    {     
        stateMachine.transitionTo(States::TRANSMIT_DATA);
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
