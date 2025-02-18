#include <Adafruit_AHTX0.h>
#include <Arduino.h>
#include <ArduinoHA.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#include "OTAHandler.h"
#include "secrets.h"
#include "thermistor.h"

#define HOSTNAME "bath_fan"
#define SSR_PIN D5
#define BUTTON_PIN 13
#define LED D4
#define SDA_PIN D2
#define SCL_PIN D1

// Вхідний пін для термістора
const int THERMISTORPIN = A0;

bool connected = false;

Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;

WiFiClient client;

HADevice device(HOSTNAME);
HAMqtt mqtt(client, device);
// Unique ID
HASwitch fanSwitch("fan_switch");
HASensorNumber wifiRssi("wifiRssi_Bath", HASensorNumber::PrecisionP0);
HASensorNumber bathTemp("Bath_temp", HASensorNumber::PrecisionP2);
HASensorNumber bathHum("Bath_hum", HASensorNumber::PrecisionP2);
HASensorNumber nodeTemp("node_temp", HASensorNumber::PrecisionP2);

// Створюємо об'єкт кнопки
// button btn(BUTTON_PIN);

void setupWiFi() {
  // template function
  delay(10);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void onSwitchCommand(bool state, HASwitch *sender) {
  // SSR only, instead MSR
  // digitalWrite(SSR_PIN, (state ? HIGH : LOW));
  digitalWrite(SSR_PIN, (state ? LOW : HIGH));  // інвертна логіка
  // ssr підключений "-" до вивода, інший на +3.3В
  sender->setState(state);  // report state back to the Home Assistant
}

void onMqttConnected() {
  digitalWrite(LED, HIGH);
  connected = true;
}

void onMqttDisconnected() {
  Serial.println("Disconnected from the broker!");
  connected = false;
}

void onMqttStateChanged(HAMqtt::ConnectionState state) {
  Serial.print("MQTT state changed to: ");
  Serial.println(static_cast<int8_t>(state));
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");
  Wire.begin(SDA_PIN, SCL_PIN);

  setupWiFi();

  // I2C scaner
  // Serial.println("Сканую I2C пристрої...");
  // for(uint8_t address = 1; address < 127; address++) {
  //   Wire.beginTransmission(address);
  //   if(Wire.endTransmission() == 0) {
  //     Serial.print("Знайдено пристрій на 0x");
  //     Serial.println(address, HEX);
  //   }
  // }

  Serial.println("Ініціалізація AHT20...");
  if(!aht.begin()) {
    Serial.println("Не вдалося знайти AHT20! Перевір з'єднання.");
    while(1) delay(10);
  }
  Serial.println("AHT20 знайдено!");

  // Логування розміру флеш-пам'яті
  uint32_t flashSize = ESP.getFlashChipRealSize();
  Serial.print("Flash size: ");
  Serial.print(flashSize);
  Serial.println(" bytes");

  // set device's details (optional)
  device.setName(HOSTNAME);
  device.setSoftwareVersion("1.0.0");

  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, HIGH);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

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

  wifiRssi.setIcon("mdi:wifi");
  wifiRssi.setName("WIFI RSSI");
  wifiRssi.setUnitOfMeasurement("dBm");

  // handle switch state
  fanSwitch.onCommand(onSwitchCommand);
  mqtt.onConnected(onMqttConnected);
  mqtt.onDisconnected(onMqttDisconnected);
  mqtt.onStateChanged(onMqttStateChanged);

  mqtt.begin(BROKER_ADDR, MQTT_USERNAME, MQTT_PASSWORD);
  device.enableSharedAvailability();
  device.enableLastWill();

  // Ініціалізація OTA з паролем
  setupOTA(HOSTNAME, OTA_PASSWORD);
}

unsigned long lastUpdateAt = 0;             // Змінна для зберігання часу останнього оновлення
const unsigned long updateInterval = 5000;  // Інтервал оновлення в мілісекундах (3000 мс = 3 секунди)
unsigned int wifi_fail_counter = 0;
const unsigned int wifi_fail_triger = 300000;  // при кількості спроб реконнест
void loop() {
  // Перевірка WiFi з'єднання
  if(!connected && WiFi.status() == WL_CONNECTED) {
    // not mqtt and connected to wifi
    Serial.println("WiFi OK, mqtt NOK");
    digitalWrite(LED, (millis() / 1000) % 2);
  }
  // Перевірка WiFi з'єднання
  if(WiFi.status() != WL_CONNECTED && wifi_fail_counter > wifi_fail_triger) {
    Serial.println("WiFi lost, trying to reconnect...");
    setupWiFi();
  }
  if(WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED, LOW);
    wifi_fail_counter++;
    Serial.print("WiFi lost, counter=");
    Serial.println(wifi_fail_counter);
    return;
  }

  mqtt.loop();
  // TODO: set another port
  ArduinoOTA.handle();

  // Перевірка натискання кнопки
  // if(btn.click()) {
  //   // Зміна стану перемикача
  //   // TODO: in esp-01 on pin 3 (RX) there is a problem. switch flashing permanently.
  //   // робити тригер щоб Serial.end(); через якийсь час.
  //   // продумати, щоб можна було підключити serial до цього часу.
  //   // onSwitchCommand(!fanSwitch.getCurrentState(), &fanSwitch);
  // }

  // Перевіряємо, чи минув інтервал оновлення
  if(millis() - lastUpdateAt > updateInterval) {
    lastUpdateAt = millis();
    // Моніторинг рівня WiFi сигналу
    int8_t rssi = WiFi.RSSI();
    wifiRssi.setValue(rssi);

    // float temperature = getTemperature(THERMISTORPIN);
    // Serial.print("Temperature: ");
    // Serial.print(temperature);
    // Serial.println(" *C");
    nodeTemp.setValue(getTemperature(THERMISTORPIN));

    aht.getEvent(&humidity, &temp);  // Зчитування даних
    Serial.print("temperature: ");
    Serial.print(temp.temperature);
    Serial.println("°C");
    Serial.print("humidity: ");
    Serial.print(humidity.relative_humidity);
    Serial.println("%");
    bathTemp.setValue(temp.temperature);
    bathHum.setValue(humidity.relative_humidity);
  }
}
