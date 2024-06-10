#include <ESP8266WiFi.h>
#include "ha_functions.h"
// #include "ha_functions.h"

// float recievedNumber = 10.0f;
// void onNumberCommand(HANumeric number, HANumber* sender)
// {
//     if (!number.isSet()) {
//         // the reset command was send by Home Assistant
//     } else {
//         // you can do whatever you want with the number as follows:
//         recievedNumber = number.toFloat();
//         Serial.print("recievedNumber=");
//         Serial.println(recievedNumber);
//     }

//     // sender->setState(recievedNumber); // report the selected option back to the HA panel
// }

void init_ha(HADevice &device, HAMqtt &mqtt, HASensorNumber &currentTemp, HABinarySensor &heaterOnHA, HANumber &targetTemp)
{
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
    device.setName("Water Boiler");
    device.setSoftwareVersion("1.1.0");

    // configure sensors
    currentTemp.setIcon("mdi:thermometer");
    currentTemp.setName("Water Temp");
    currentTemp.setUnitOfMeasurement("°C");

    heaterOnHA.setIcon("mdi:water-boiler");
    heaterOnHA.setName("heater status");

    targetTemp.setIcon("mdi:thermometer");
    targetTemp.setName("target temp");
    targetTemp.setUnitOfMeasurement("°C");
    targetTemp.setDeviceClass("temperature");
    targetTemp.setMin(20.0f);
    targetTemp.setMax(100.0f);
    // targetTemp.setMode(HANumber::ModeBox);
    targetTemp.setMode(HANumber::ModeSlider);
    // remember value and send after on/off/on device
    targetTemp.setRetain(true);
    // targetTemp.onCommand(onNumberCommand);

    mqtt.begin(BROKER_ADDR, MQTT_USERNAME, MQTT_PASSWORD);
    // mqtt.setDiscoveryPrefix("homeassistant");
    // mqtt.setDataPrefix("aha");
    device.enableSharedAvailability();
    device.enableLastWill();
}

// void numberWraper(float &number, HANumber &haNumber){
//     Serial.print("wraper recievedNumber=");
//     Serial.println(recievedNumber);
//     // handle command from the HA panel
//     haNumber.onCommand(onNumberCommand);
//     number = recievedNumber;
// }
