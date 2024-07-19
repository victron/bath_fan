#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoHA.h>
#include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME280.h>

#include "secrets.h"
#include "OTAHandler.h"
#include "thermistor.h"

#define SSR_PIN 14
#define RELAY_PIN 12

const int SSR_relay_delay = 2000;

// Вхідний пін для термістора
const int THERMISTORPIN = A0;

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
// TODO: humidity
// HASensorNumber currentHum("Bath humidity", HASensorNumber::PrecisionP1);
HASwitch fanSwitch("fan_switch");
HABinarySensor fanOnHA("fan_on");
HASensorNumber nodeTemp("node_temp", HASensorNumber::PrecisionP2);

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
    Serial.begin(115200);
    Serial.println("Booting...");

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
    Serial.println("Connected to the network:::::");

    // Логування розміру флеш-пам'яті
    uint32_t flashSize = ESP.getFlashChipRealSize();
    Serial.print("Flash size: ");
    Serial.print(flashSize);
    Serial.println(" bytes");

    // set device's details (optional)
    device.setName("bath_fan");
    device.setSoftwareVersion("1.0.0");

    pinMode(SSR_PIN, OUTPUT);
    digitalWrite(SSR_PIN, LOW);
    // relay high level trigger
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    // set icon (optional)
    fanSwitch.setIcon("mdi:fan");
    fanSwitch.setName("Bath Fan");

    // configure sensors
    nodeTemp.setIcon("mdi:thermometer");
    nodeTemp.setName("Node Temp");
    nodeTemp.setUnitOfMeasurement("°C");

    fanOnHA.setIcon("mdi:fan");
    fanOnHA.setName("Fan status");

    // handle switch state
    fanSwitch.onCommand(onSwitchCommand);

    mqtt.begin(BROKER_ADDR, MQTT_USERNAME, MQTT_PASSWORD);
    device.enableSharedAvailability();
    device.enableLastWill();

    // Ініціалізація OTA з паролем
    setupOTA("bath_fan", OTA_PASSWORD);
}

unsigned long lastUpdateAt = 0;            // Змінна для зберігання часу останнього оновлення
const unsigned long updateInterval = 3000; // Інтервал оновлення в мілісекундах (3000 мс = 3 секунди)
void loop()
{
    mqtt.loop();
    ArduinoOTA.handle();

    // You can also change the state at runtime as shown below.
    // This kind of logic can be used if you want to control your switch using a button connected to the device.
    // led.setState(true); // use any state you want

    // Перевіряємо, чи минув інтервал оновлення
    if (millis() - lastUpdateAt > updateInterval)
    {
        float temperature = getTemperature(THERMISTORPIN);
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" *C");
        nodeTemp.setValue(temperature);

        lastUpdateAt = millis();
    }
}
