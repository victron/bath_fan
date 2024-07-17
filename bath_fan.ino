#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ArduinoHA.h>
#include <Wire.h>

#include "secrets.h"

#define SSR_PIN 14
#define RELAY_PIN 12

const int SSR_relay_delay = 2000;

// globals for temp sensor
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
// TODO: humidity
// HASensorNumber currentHum("Bath humidity", HASensorNumber::PrecisionP1);
HASwitch fanSwitch("fan_switch");
HABinarySensor fanOnHA("fan_on");

void onSwitchCommand(bool state, HASwitch *sender)
{
    digitalWrite(SSR_PIN, HIGH);
    delay(SSR_relay_delay);
    digitalWrite(RELAY_PIN, (state ? HIGH : LOW));
    delay(SSR_relay_delay);
    digitalWrite(SSR_PIN, LOW);

    sender->setState(state); // report state back to the Home Assistant
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting...");

    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    // connect to wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500); // waiting for the connection
    }
    Serial.println();
    Serial.println("Connected to the network");

    // set device's details (optional)
    device.setName("bath_fan");
    device.setSoftwareVersion("1.0.0");

    pinMode(SSR_PIN, OUTPUT);
    digitalWrite(SSR_PIN, LOW);
    // rellay high level triger
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    // set icon (optional)
    fanSwitch.setIcon("mdi:fan");
    fanSwitch.setName("Bath Fan");

    // handle switch state
    fanSwitch.onCommand(onSwitchCommand);

    mqtt.begin(BROKER_ADDR, MQTT_USERNAME, MQTT_PASSWORD);
    device.enableSharedAvailability();
    device.enableLastWill();
}

void loop()
{
    mqtt.loop();

    // You can also change the state at runtime as shown below.
    // This kind of logic can be used if you want to control your switch using a button connected to the device.
    // led.setState(true); // use any state you want
}