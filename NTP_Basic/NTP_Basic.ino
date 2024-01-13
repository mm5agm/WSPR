/* 
Author Colin Campbell MM5AGM
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/
/*
A simple test program to get date and time from an NTP server
  There is no formatting of the date and time, that will be done in a later program, NPT_Time_With_RTC.ino
  The NTP library is https://github.com/SensorsIot/NTPtimeESP 
  NTP servers allow you to get the time every 4 seconds but 1 or twice a day to update a real time clock 
  should be sufficient. If you query more often you risk being banned from that server.
*/
#include <NTPtimeESP.h>

const char* NTP_Server = "uk.pool.ntp.org";  // pick an ntp server in your area
NTPtime NTPch(NTP_Server);  //make an instance to work with
strDateTime NTPdateTime;           //strDateTime is declared in NTPtimeESP.h as a type
const char* ssid = "*********";        // SSID of your Wifi network
const char* password = "******";  // Password of your wifi network
int failCount = 60;                // maximun number of times to attempt to connect to wi-fi. Attempts are 500Ms appart
int BAUDRATE = 9600;
#define DST_OFFSET 0     //  1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment;
#define TIME_ZONE +0.0f  // used in NTP time calculation. UTC time zone difference in regards to UTC (floating point number)

/********************************************
***    Initialse and connect to Wi-Fi     ***
*********************************************/
void initialiseWiFi() {  // will attempt to connect to the local router/hub
  int attempts = 0;
  WiFi.mode(WIFI_OFF);  //Prevents reconnection issue (taking too long to connect)
  WiFi.mode(WIFI_STA);  //This line hides the viewing of ESP as wifi hotspot
  Serial.println("Connecting to Wi-Fi   ");
  Serial.print(" Using ssid "); Serial.print(ssid); Serial.print("  Password "); Serial.println(password);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { // keep coming back to here until either connected or tried 61 times
    // will be stuck in this loop till connected or timeout at about 30 seconds for failCount = 60
    attempts++;
    Serial.print("Attempts to connect = ");
    Serial.println(attempts);
    if (attempts > failCount) {
      Serial.print("attempts = ");
      Serial.println(attempts);
      Serial.println("Are you sure you have the correct ssid and password?");
      while (1) {};  // program will stick here to let user read display
    }
    delay(500);
  }
  // If we get to here we have connected to wifi
  Serial.println();
  Serial.print("- Connected to: ");
  Serial.println(ssid);
  Serial.print("- IP address: ");
  Serial.println(WiFi.localIP());
}

/**********************************************************
***    Print the date and time to the serial monitor    ***
***********************************************************/
void showDate(strDateTime now) {
  Serial.print(now.day);
  Serial.print("/");
  Serial.print(now.month);
  Serial.print("/");
  Serial.print(now.year);
  Serial.print("    ");
  Serial.print(now.hour);
  Serial.print(":");
  Serial.print(now.minute);
  Serial.print(":");
  Serial.print(now.second);
  Serial.println();
}
/**************************************************************************
***   The setup function runs just once at the start of the program     ***
***************************************************************************/
void setup() {
  Serial.begin(BAUDRATE);
  Serial.println("Starting Up");
  initialiseWiFi();  // If we don't get an internet connection we'll be stuck in the trying to connect loop
}
/**************************************************************************
***   The loop() function runs continuously after setup.                ***
***   When it gets to the end of the loop it goes back to the beginning ***
***   of loop()                                                         ***
***************************************************************************/
void loop() {
  do {
    NTPdateTime = NTPch.getNTPtime(TIME_ZONE, DST_OFFSET); // get the date and time 
    delay(1);
  } while (!NTPdateTime.valid);  // keep trying till you get a valid time
  if (NTPdateTime.valid) {
    showDate(NTPdateTime);
  }
  delay(10000);  // the keepers of the NTP servers will ban you if you request the time more often than once every 4 seconds so we'll retry after 10 seconds
}
