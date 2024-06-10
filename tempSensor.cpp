#include <Arduino.h>
#include "tempSensor.h"


float calcTemp(int ThermistorPin) {
    // int ThermistorPin = 0;
    // int Vo;
    // correction to get 3.2v as max on adc
    float Rcorr = 300;
    float R1 = 7500 - Rcorr ;
    // float logR2, R2, T, Tc;

    // koef calculation based on values 4894: 41, 3000: 54, 1364: 78 
    // https://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
    // float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
    float c1 = 0.7430731362e-03, c2 = 3.029239792e-04, c3 = -2.176284401e-07;

    int Vo = analogRead(ThermistorPin);
      // R2 = R1 * (1023.0 / (float)Vo - 1.0);
      float R2 = Vo * R1 / (1023 - Vo);

      float logR2 = log(R2);
      float T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
      return T - 273.15;
}