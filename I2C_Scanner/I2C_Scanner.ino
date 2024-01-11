/* 

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/
/*

A program to scan the I2C bus addresses and report what's found

*/
#include <Wire.h>

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  while (!Serial)
  {
    // wait for serial port to connect. Needed for native USB port only
  }
}

void loop()
{
  int error, address;
  int numDevices;

  Serial.println("Scanning...");

  numDevices = 0;
  for (address = 1; address < 127; address++) // I2C addresses start at 1 and go up to 127
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      numDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (numDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.print("Finished Scanning. Number of devices found = ");
    Serial.println(numDevices);
    Serial.println(" Fresh scan starting in 5 seconds");

  delay(5000); // wait 5 seconds for next scan
}
