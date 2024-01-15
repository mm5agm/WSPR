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
A simple test program to get date and time from an NTP server and update a real time clock
  Unfortuneately the 2 libraries I use to get time define time differently
  The NTP library has days 0 to 6, with 0 = Sunday and 6 = Saturday
  The RTC library has days 1 to 7, with 1 = Sunday and 7 = Saturday
  NTP library is https://github.com/SensorsIot/NTPtimeESP 
  Real Time Clock library is Adafruit 2.1.3 for RTC like DS3231
*/

#include <NTPtimeESP.h>
#include <RTClib.h>  // Adafruit 2.1.3 for RTC like DS3231

const char* ssid = "*******";                                                  // SSID of your Wifi network
const char* password = "******";                                            // Password of your wifi network
RTC_DS3231 rtc;                                                                  // create an instance of the real time clock
const char* weekDays[] = { "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat" };  // weekDays[0] = "Sun",  weekDays[6] = "Sat"
//NTP Server:
const char* NTP_Server = "uk.pool.ntp.org";  // pick an ntp server in your area
NTPtime NTPch(NTP_Server);                   //make an instance of an NTP server to work with
strDateTime NTPdateTime;                     //strDateTime is declared in NTPtimeESP.h as a type
int failCount = 60;                          // maximun number of times to attempt to connect to wi-fi. Attempts are 500Ms appart
int BAUDRATE = 9600;
#define DST_OFFSET 0     //  1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment;
#define TIME_ZONE +0.0f  // used in NTP time calculation. UTC time zone difference in regards to UTC (floating point number)

/********************************************************************************************
*** padZero - print a "0" in front of a single digit number in the Serial Monitor Output  ***
*** As I want to use this inside other functions, I can't declare it as "void" so I       ***
*** declare it as returning a pointer to a character and send back ""                                 ***
*********************************************************************************************/
char* padZero(int aNumber) {
  if (aNumber < 10) {
    Serial.print("0");
    Serial.print(aNumber);
  } else {
    Serial.print(aNumber);
  }
  return "";
}
/********************************************
***    Initialse and connect to Wi-Fi     ***
*********************************************/
void initialiseWiFi() {  // will attempt to connect to the local router/hub
  int attempts = 0;
  WiFi.mode(WIFI_OFF);  //Prevents reconnection issue (taking too long to connect)
  WiFi.mode(WIFI_STA);  //This line hides the viewing of ESP as wifi hotspot

  Serial.println("Connecting to Wi-Fi   ");
  Serial.print(" Using ssid ");
  Serial.print(ssid);
  Serial.print("  Password ");
  Serial.println(password);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {  // keep coming back to here until either connected or tried 61 times
    attempts++;
    Serial.print("Attempts to connect = ");
    Serial.println(attempts);
    if (attempts > failCount) {
      Serial.print("attempts = ");
      Serial.println(attempts);
      Serial.println("Are you sure you have the correct ssid and password?");
      while (1) {};  // program will stick here to let user read display. User will need to crash out
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
/******************************************************************
*** Show, on the serial port monitor, the NTP date and time     ***
*** NTP and RTC use a different definition of time so need      ***
*** different functions to print                                ***
*******************************************************************/
void serialShowDateTimeNTP(strDateTime ntpDateTime) {
  Serial.print("  NTP queried - ");
  Serial.print(weekDays[ntpDateTime.dayofWeek - 1]);  //doesn't work. It prints the password first!
  Serial.print("  ");
  Serial.print(padZero(ntpDateTime.day));
  Serial.print("/");
  Serial.print(padZero(ntpDateTime.month));
  Serial.print("/");
  Serial.print(padZero(ntpDateTime.year));
  Serial.print("  ");
  Serial.print(padZero(ntpDateTime.hour));
  Serial.print(":");
  Serial.print(padZero(ntpDateTime.minute));
  Serial.print(":");
  Serial.print(padZero(ntpDateTime.second));
  Serial.print("   ");
}
/******************************************************************
*** Show, on the serial port monitor, the RTC date and time     ***
*** NTP and RTC use a different definition of time so need      ***
*** different functions to print                                ***
*******************************************************************/
void serialShowDateTimeRTC(DateTime rtcDateTime) {
  Serial.print("RTC - ");
  Serial.print(weekDays[rtcDateTime.dayOfTheWeek()]);
  Serial.print("  ");
  Serial.print(padZero(rtcDateTime.day()));
  Serial.print("/");
  Serial.print(padZero(rtcDateTime.month()));
  Serial.print("/");
  Serial.print(padZero(rtcDateTime.year()));
  Serial.print("  ");
  Serial.print(padZero(rtcDateTime.hour()));
  Serial.print(":");
  Serial.print(padZero(rtcDateTime.minute()));
  Serial.print(":");
  Serial.println(padZero(rtcDateTime.second()));
}
/*****************************************
*** Update the RTC from the NTP server ***
******************************************/
void updateRTC() {
  do {
    NTPdateTime = NTPch.getNTPtime(TIME_ZONE, DST_OFFSET);  //1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment; ( not tested by me)
    delay(1);
  } while (!NTPdateTime.valid);  // keep trying till you get a valid time
                                 // setTime is from the <TimeLib.h> library
                                 // setTime(NTPdateTime.hour, NTPdateTime.minute, NTPdateTime.second, NTPdateTime.day, NTPdateTime.month, NTPdateTime.year);  // sets the systemtime to the time returned from NTP
                                 // rtc.adjust comes from the adafruit rtc lib
  rtc.adjust(DateTime(NTPdateTime.year, NTPdateTime.month, NTPdateTime.day, NTPdateTime.hour, NTPdateTime.minute, NTPdateTime.second));
  serialShowDateTimeNTP(NTPdateTime);
}
/*****************************************
*** Initialise the RTC from the NTP server ***
******************************************/
void initialiseRTC() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);  // stay here until rtc found - if it's not found you're here forever
  }
  updateRTC();  // now set it to correct time
}
/**************************************************************************
***   The setup function runs just once at the start of the program     ***
***************************************************************************/
void setup() {
  Serial.begin(BAUDRATE);
  Serial.println("Starting Up");
  initialiseWiFi();  // If we don't get an internet connection we'll be stuck in the trying to connect loop
  initialiseRTC();
}
/**************************************************************************
***   The loop() function runs continuously after setup.                ***
***   When it gets to the end of the loop it goes back to the beginning ***
***************************************************************************/
void loop() {
  DateTime now = rtc.now();
  if ((now.minute() % 2 == 0) && (now.second() == 0)) {  //update the rtc from ntp every 2 mins
    updateRTC();
    Serial.print("  RTC update at  ");
    serialShowDateTimeRTC(rtc.now());
  }
  serialShowDateTimeRTC(now);  // Show user date and time from RTC
  delay(1000);                 //wait 1 second before getting the time from the rtc, this will mean our output is a bit behind real time
}