#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoHA.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "secrets.h"
#include "OTAHandler.h"
#include "thermistor.h"
#include "button.h"

#define HOSTNAME "bath_fan"
#define SSR_PIN 14
#define RELAY_PIN 12
#define BUTTON_PIN 13
// #define SEALEVELPRESSURE_HPA (1013.25)

const int SSR_relay_delay = 2000;

// Вхідний пін для термістора
const int THERMISTORPIN = A0;

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HASwitch fanSwitch("fan_switch");
// HABinarySensor fanOnHA("fan_on");
HASensorNumber nodeTemp("node_temp", HASensorNumber::PrecisionP2);

Adafruit_BME280 bme;
HASensorNumber bathTemp("bath_temp", HASensorNumber::PrecisionP2);
HASensorNumber bathHum("bath_hum", HASensorNumber::PrecisionP2);
HASensorNumber bathPres("bath_pres", HASensorNumber::PrecisionP2);
HASensorNumber wifi_rssi("WiFi Power", HASensorNumber::PrecisionP0);

// Створюємо об'єкт кнопки
button btn(BUTTON_PIN);

void setupWiFi()
{
    delay(10);
    Serial.println();
    Serial.print("connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.hostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

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

    setupWiFi();

    // Логування розміру флеш-пам'яті
    uint32_t flashSize = ESP.getFlashChipRealSize();
    Serial.print("Flash size: ");
    Serial.print(flashSize);
    Serial.println(" bytes");

    // set device's details (optional)
    device.setName(HOSTNAME);
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

    bathTemp.setIcon("mdi:thermometer");
    bathTemp.setName("Bath Temp");
    bathTemp.setUnitOfMeasurement("°C");

    bathHum.setIcon("mdi:water-percent");
    bathHum.setName("Bath Humidity");
    bathHum.setUnitOfMeasurement("%");

    bathPres.setIcon("mdi:gauge");
    bathPres.setName("Bath Pressure");
    bathPres.setUnitOfMeasurement("hPa");

    wifi_rssi.setIcon("mdi:gauge");
    wifi_rssi.setName("WiFi rsi");
    wifi_rssi.setUnitOfMeasurement("dBm");

    // fanOnHA.setIcon("mdi:fan");
    // fanOnHA.setName("Fan status");

    // handle switch state
    fanSwitch.onCommand(onSwitchCommand);

    mqtt.begin(BROKER_ADDR, MQTT_USERNAME, MQTT_PASSWORD);
    device.enableSharedAvailability();
    device.enableLastWill();

    // Підключення до BME280
    if (!bme.begin(0x76))
    {
        Serial.println("BME280 not found!");
        while (1)
            ;
    }

    // Ініціалізація OTA з паролем
    setupOTA("bath_fan", OTA_PASSWORD);
}

unsigned long lastUpdateAt = 0;            // Змінна для зберігання часу останнього оновлення
const unsigned long updateInterval = 3000; // Інтервал оновлення в мілісекундах (3000 мс = 3 секунди)
unsigned int wifi_fail_counter = 0;
const unsigned int wifi_fail_triger = 300000; // при кількості спроб реконнест
void loop()
{
    // Перевірка WiFi з'єднання
    if (WiFi.status() != WL_CONNECTED && wifi_fail_counter > wifi_fail_triger)
    {
        Serial.println("WiFi lost, trying to reconnect...");
        setupWiFi();
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        wifi_fail_counter++;
        Serial.print("WiFi lost, counter=");
        Serial.println(wifi_fail_counter);
        return;
    }

    // Перевірка натискання кнопки
    if (btn.click())
    {
        // Зміна стану перемикача
        onSwitchCommand(!fanSwitch.getCurrentState(), &fanSwitch);
    }

    mqtt.loop();
    ArduinoOTA.handle();

    // Перевіряємо, чи минув інтервал оновлення
    if (millis() - lastUpdateAt > updateInterval)
    {
        lastUpdateAt = millis();
        // Моніторинг рівня WiFi сигналу
        int32_t rssi = WiFi.RSSI();
        wifi_rssi.setValue((float)0.0);

        float temperature = getTemperature(THERMISTORPIN);
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" *C");
        nodeTemp.setValue(temperature);

        // Зчитування даних з BME280
        float temperature_bme = bme.readTemperature();
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F;

        bathTemp.setValue(temperature_bme);
        bathHum.setValue(humidity);
        bathPres.setValue(pressure);
    }
}
