/*
    PoolGuy project; main firmware file

    Hardware configuration:
        MCU:                    Arduino MKR 1010 WiFi (SAMD21)
        PH SENSOR:              GROVE E201C-BLUE    (Connected to D8)
        TEMP SENSOR:            DSB18B20 ONEWIRE    (Connected to A1)
*/

#include "definitions.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include <StateMachine.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// Heap allocated globally accessible instances (Singletons)
/*E201C *phsensor = new E201C(REF_VOLTAGE, ADC_MAX_VAL_REF, PH_REF_1, PH_REF_2, PH_SENSOR_CHANNEL);
*/

OneWire oneWire(TEMP_PORT);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

const char THING_ID[] = DEVICE_ID;
const char SSID[]     = WIFI_SSID;
const char PASS[]     = WIFI_PASS;


WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
float temp;


void tempChanged()
{
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" C");
}


void setup()
{
    Serial.begin(9600);
    sensors.begin();
    sensors.setResolution(insideThermometer, 9);
    sensors.getAddress(insideThermometer, 0);

    ArduinoCloud.setThingId(THING_ID);
    ArduinoCloud.addProperty(temp, READWRITE, 60 * SECONDS, tempChanged);
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);

    setDebugMessageLevel(2);
    ArduinoCloud.printDebugInfo();
}


void loop() 
{
    sensors.requestTemperatures();
    delay(10);
    temp = sensors.getTempC(insideThermometer);
    ArduinoCloud.update();
}
