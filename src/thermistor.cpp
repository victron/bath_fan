#include "thermistor.h"
#include <Arduino.h>
#include <math.h>

// Константи для розрахунку температури
const float SERIESRESISTOR = 9940.0;     // Додатковий резистор 10k
const float THERMISTORNOMINAL = 10000.0; // Опір термістора при 25 градусах C
const float TEMPERATURENOMINAL = 25.0;   // Температура при якій опір номінальний (25 C)
const float ADC_MAX_VALUE = 1023.0 + 5;  // корекція
const float BCOEFFICIENT = 3950.0;       // Beta коефіцієнт термістора

float getTemperature(int thermistorPin)
{
    // esp8266 posible values 0..1 volts
    int adcValue = analogRead(thermistorPin); // Читаємо значення з аналогового входу
    float resistance = SERIESRESISTOR * ((ADC_MAX_VALUE / adcValue) - 1);

    float steinhart;
    steinhart = resistance / THERMISTORNOMINAL;       // (R/Ro)
    steinhart = log(steinhart);                       // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                        // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                      // Інверсія значення
    steinhart -= 273.15;                              // Перетворення в градуси Цельсія

    return steinhart;
}
