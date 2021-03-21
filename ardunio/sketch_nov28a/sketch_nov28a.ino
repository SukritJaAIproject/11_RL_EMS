#include "EmonLib.h"
// Include Emon Library
EnergyMonitor emon1;
// Create an instance
void setup()
{
  Serial.begin(9600);

  emon1.current(34, 12);             // Current: input pin, calibration.
//  emon1.current(34, 111.1);  
}

void loop()
{
double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  Serial.println(analogRead(34));
  Serial.print("Power = ");
  Serial.println(1.732*Irms*234.7);           // Apparent power
  Serial.print("Irms = ");
  Serial.println(Irms);             // Irms
  delay(5000);
}
