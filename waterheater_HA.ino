#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoHA.h>
#include <Wire.h>

#include "ha_functions.h"
// #include "oled.h"
#include "tempSensor.h"
#include "logic.h"

// globals for temp sensor
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HASensorNumber currentTemp("water_temp", HASensorNumber::PrecisionP1);
HABinarySensor heaterOnHA("heater_on");
HANumber targetTemp("target_temp", HANumber::PrecisionP0);

bool heaterOn = false;

// temp const
float haTemp = 20.0f;
const float hysteresis = 3;
const float maxTemp = 80.0f;

// globals for HA
// HAMqtt mqtt;

// float recievedNumber = 10.0f;
void onNumberCommand(HANumeric number, HANumber *sender)
{
    if (!number.isSet())
    {
        // the reset command was send by Home Assistant
    }
    else
    {
        // you can do whatever you want with the number as follows:
        haTemp = number.toFloat();
        // protector for overheating

        if (haTemp > maxTemp)
        {
            haTemp = maxTemp;
        }

        Serial.print("recievedNumber=");
        Serial.println(haTemp);
    }

    sender->setState(haTemp); // report the selected option back to the HA panel
}

void setup()
{
    // pins
    // ledid - board power id
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // relay + SSR
    // initial switch off
    // SSR
    pinMode(15, OUTPUT);
    digitalWrite(15, LOW);
    // relay
    pinMode(14, OUTPUT);
    digitalWrite(14, LOW);

    Serial.begin(115200);
    while (!Serial)
    {
        delay(100);
    }

    init_ha(device, mqtt, currentTemp, heaterOnHA, targetTemp);

    //   short delay before measurements
    delay(1000);
    // handle command from the HA panel
    // numberWraper(haTemp, targetTemp);
    targetTemp.onCommand(onNumberCommand);
    // numberWraper(haTemp, targetTemp);
    Serial.print("initial haTemp=");
    Serial.println(haTemp);
}

unsigned long lastUpdateAt = 0;
void loop()
{
    mqtt.loop();

    if ((millis() - lastUpdateAt) > 3000)
    {
        // logic
        float sensorTemp = calcTemp(0);
        heaterOn = heaterLogic2(sensorTemp, haTemp, hysteresis, heaterOn);
        Serial.print("haTemp=");
        Serial.println(haTemp);
        Serial.print("sensorTemp=");
        Serial.println(sensorTemp);
        Serial.print("heaterOn=");
        Serial.println(heaterOn);

        currentTemp.setValue(sensorTemp);
        heaterOnHA.setState(heaterOn);
        // targetTemp.setState(25.25f);

        lastUpdateAt = millis();
        // you can reset the sensor as follows:
        // analogSensor.setValue(nullptr);
    }
}
