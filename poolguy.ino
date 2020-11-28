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

/* Constants */
const char THING_ID[] = DEVICE_ID;
const char SSID[]     = WIFI_SSID;
const char PASS[]     = WIFI_PASS;

/* Arduino IoT Cloud properties */
float temp;

/* Heap allocated globally accessible instances (Singletons) */
//E201C *phSensor = new E201C(REF_VOLTAGE, ADC_MAX_VAL_REF, PH_REF_1, PH_REF_2, PH_SENSOR_CHANNEL);
DS18B20 *tempSensor = new DS18B20(TEMP_PORT_GROUP, PORT_PA16);
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);


void tempChanged()
{
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" C");
}


void setup()
{
    Serial.begin(112500);

    ArduinoCloud.setThingId(THING_ID);
    ArduinoCloud.addProperty(temp, READWRITE, 60 * SECONDS, tempChanged);
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);

    setDebugMessageLevel(2);
    ArduinoCloud.printDebugInfo();
}


uint32_t last_iter;
void loop() 
{
    if (millis() - last_iter > STDBY_TIME_MS)
    {
        temp = tempSensor->GetTemperature('C');
        Serial.println(temp);
        last_iter = millis();
        ArduinoCloud.update();
    }
}