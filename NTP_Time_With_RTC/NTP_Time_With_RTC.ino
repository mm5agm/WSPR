/*****************************************************************************************************************
***                     Author Colin Campbell MM5AGM             mm5agm@outlook.com                            ***
*** This program is free software: you can redistribute it and/or modify it under the terms of the GNU         ***
*** General Public License as published by the Free Software Foundation, either version 3 of the License,      ***
*** or (at your option) any later version.                                                                     ***
***                                                                                                            ***
*** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without          ***
*** even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                          ***
*** See the GNU General Public License for more details.                                                       ***
******************************************************************************************************************/

/*****************************************************************************************************************
*** This is the third of a series of programs that culminate in a WSPR beacon transmitter. Each program builds ***
*** on the previous one by adding 1 component or more code. This program gets the date and time from an NTP    ***
*** server and uses this to update a real time clock. Output is formatted date and time to the serial monitor. ***
*** Unfortuneately the 2 libraries I use to get time define time differently. The program shows date and time  ***
*** formatted from NTP and RTC.                                                                                ***
*** The NTP library is https://github.com/SensorsIot/NTPtimeESP  NTP servers allow you to get the              ***
*** time approximately every 4 seconds. If you query more often than every 4 seconds you risk being banned     ***
*** from that server. The library used to get the time returns the number of seconds since 1st January 1970.   ***
*** This means that the time that's reported can be up to 1 second out.                                        ***
*** The RTC library has days 0 to 6, with 0 = Sunday and 6 = Saturday                                          ***
*** The NTP library has days 1 to 7, with 1 = Sunday and 7 = Saturday                                         ***
*** NTP library is https://github.com/SensorsIot/NTPtimeESP                                                    ***
*** Real Time Clock library is Adafruit 2.1.3 for RTC like DS3231                                              ***
*** Hardware required = ESP32 and Real Time Clock DS3231                                                       ***
******************************************************************************************************************/

#include <NTPtimeESP.h>  // https://github.com/SensorsIot/NTPtimeESP
#include <RTClib.h>      // Adafruit 2.1.3 for RTC like DS3231

const char* ssid = "*******";                                                    // SSID of your Wifi network
const char* password = "******";                                                 // Password of your wifi network
RTC_DS3231 rtc;                                                                  // create an instance of the real time clock
const char* weekDays[] = { "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat" };  // RTC weekDays[0] = "Sun", weekDays[6] = "Sat"
//NTP Server:
const char* NTP_Server = "uk.pool.ntp.org";  // pick an ntp server in your area
NTPtime NTPch(NTP_Server);                   //make an instance of an NTP server to work with
strDateTime NTPdateTime;                     //strDateTime is declared in NTPtimeESP.h as a type
int failCount = 60;                          // maximun number of times to attempt to connect to wi-fi. Attempts are 500Ms appart
int BAUDRATE = 9600;
#define DST_OFFSET 0     //  1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment;
#define TIME_ZONE +0.0f  // used in NTP time calculation. UTC time zone difference in regards to UTC (floating point number)

/************************** [ padZero ] ************************************************
*** Print a "0" in front of a single digit number in the Serial Monitor Output       ***
****************************************************************************************/
void padZero(int aNumber) {
  if (aNumber < 10) {
    Serial.print("0");
    Serial.print(aNumber);
  } else {
    Serial.print(aNumber);
  }
}

/******************************************************************************************************************************
***                             Initialse and connect to Wi-Fi                                                              ***
*******************************************************************************************************************************/
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
/*************************************************************************
*** Show, on the serial port monitor, the NTP date and time            ***
*** RTC and Time (NTP) use a different definition of time so need      ***
*** different functions to print                                       ***
**************************************************************************/
void serialShowDateTimeNTP(strDateTime ntpDateTime) {
  int i;
  Serial.print("  NTP queried - ");
  Serial.print(weekDays[ntpDateTime.dayofWeek - 1]);  //NTP library has days 1 to 7,  Sunday to Saturday
  Serial.print("  ");
  i = ntpDateTime.day;
  padZero(i);
  Serial.print("/");
  i = ntpDateTime.month;
  padZero(i);
  Serial.print("/");
  i = ntpDateTime.year;
  padZero(i);
  Serial.print("  ");
  i = ntpDateTime.hour;
  padZero(i);
  Serial.print(":");
  i = ntpDateTime.minute;
  padZero(i);
  Serial.print(":");
  i = ntpDateTime.second;
  padZero(i);
  Serial.print("   ");
}
/******************************************************************
*** Show, on the serial port monitor, the RTC date and time     ***
*** NTP and RTC use a different definition of time so need      ***
*** different functions to print                                ***
*******************************************************************/
void serialShowDateTimeRTC(DateTime rtcDateTime) {
  int i;
  Serial.print("RTC - ");
  Serial.print(weekDays[rtcDateTime.dayOfTheWeek()]);  //RTC library has days 0 to 6 Sunday to Saturday
  Serial.print("  ");
  i = rtcDateTime.day();
  padZero(i);
  Serial.print("/");
  i = rtcDateTime.month();
  padZero(i);
  Serial.print("/");
  i = rtcDateTime.year();
  padZero(i);
  Serial.print("  ");
  i = rtcDateTime.hour();
  padZero(i);
  Serial.print(":");
  i = rtcDateTime.minute();
  padZero(i);
  Serial.print(":");
  i = rtcDateTime.second();
  padZero(i);
  Serial.println();
}
/*****************************************
*** Update the RTC from the NTP server ***
******************************************/
void updateRTC() {
  do {
    NTPdateTime = NTPch.getNTPtime(TIME_ZONE, DST_OFFSET);  //1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment; ( not tested by me)
    delay(1);
  } while (!NTPdateTime.valid);  // keep trying till you get a valid time
   // rtc.adjust comes from the adafruit rtc lib
  rtc.adjust(DateTime(NTPdateTime.year, NTPdateTime.month, NTPdateTime.day, NTPdateTime.hour, NTPdateTime.minute, NTPdateTime.second));
  serialShowDateTimeNTP(NTPdateTime);
}
/*********************************************
*** Initialise the RTC from the NTP server ***
**********************************************/
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
