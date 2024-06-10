// #ifndef HA_FUNCTIONS_H
// #define HA_FUNCTIONS_H
// #include <ESP8266WiFi.h>
#include <ArduinoHA.h>
#include "secrets.h"

void init_ha(HADevice &device, HAMqtt &mqtt, HASensorNumber &currentTemp, HABinarySensor &heaterOnHA, HANumber &targetTemp);
// float &haTempref = NULL;
void onNumberCommand(HANumeric number, HANumber *sender);
void numberWraper(float &number, HANumber &target);

// float *haTemp = &numberValue;

// #endif