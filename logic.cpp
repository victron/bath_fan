#include <Arduino.h>
#include "logic.h"

const int SSR_relay_delay = 2000;

bool heaterLogic(float currentTemp, float targetTemp, float hysteresis, bool previousState)
// current relay levels for Relay Shield
{
    if (currentTemp < targetTemp - hysteresis)
    {
        // switch on
        digitalWrite(14, HIGH);
        return true;
    }

    if (currentTemp > targetTemp + hysteresis)
    {
        // switch off
        digitalWrite(14, LOW);
        return false;
    }

    return previousState;
}

bool heaterLogic2(float currentTemp, float targetTemp, float hysteresis, bool previousState)
// current relay levels for Relay Shield and SSR
// 14 pin relay
// 15 pin SSR
{
    if (currentTemp < targetTemp - hysteresis)
    // switch on
    {
        if (previousState == true)
        {
            return true;
        }

        digitalWrite(15, HIGH);
        delay(SSR_relay_delay);
        digitalWrite(14, HIGH);
        delay(SSR_relay_delay);
        digitalWrite(15, LOW);
        return true;
    }

    if (currentTemp > targetTemp + hysteresis)
    // switch off
    {
        if (previousState == false)
        {
            return false;
        }
        digitalWrite(15, HIGH);
        delay(SSR_relay_delay);
        digitalWrite(14, LOW);
        delay(SSR_relay_delay);
        digitalWrite(15, LOW);
        return false;
    }

    return previousState;
}