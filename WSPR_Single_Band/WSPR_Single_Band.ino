/*****************************************************************************************************************
***                         Author Colin Campbell MM5AGM         mm5agm@outlook.com                            ***                                                                        ***
*** This program is free software: you can redistribute it and/or modify it under the terms of the GNU         ***
*** General Public License as published by the Free Software Foundation, either version 3 of the License,      ***
*** or (at your option) any later version.                                                                     ***
***                                                                                                            ***
*** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without          ***
*** even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                          ***
*** See the GNU General Public License for more details.                                                       ***
******************************************************************************************************************/
/*****************************************************************************************************************
** This is the final sketch of a series that has culminated in this single band WSPR beacon transmitter.        **
** It's a work in progress and may be changed.                                                                  **
** You will need to change the ssid, password, callsign, locator, si5351 calibration factor, and select the     **
** frequency to TX. Change DST_OFFSET and TIME_ZONE if you want to display local time otherwise it's UTC        **        **                                                                                                              **
** Hardware required = ESP32, si5351 square wave generator, Real Time Clock DS3231, SSD1306 0.96inch OLED       **
** and a low pass filter for the band you are using                                                             **
** In Arduino IDE open the option “File/Preferences” and fill in the “Additional boards manager URLs” with      **
**  https://espressif.github.io/arduino-esp32/package_esp32_index.json                                          **
** Unfortunately the 2 libraries I use to manipulate time define time differently.                              **
** The RTC library has days 0 to 6, with 0 = Sunday and 6 = Saturday                                            **
** The Time library, has days 1 to 7, with 1 = Sunday and 7 = Saturday                                          **
** Time library by Paul Stoffregen                                          - install from Arduino IDE          **
** NTP library is https://github.com/SensorsIot/NTPtimeESP                  - download from github and install  **
** Real Time Clock library is Adafruit 2.1.3 for a RTC like DS3231          - install from Arduino IDE          **
** OLED library is Adafruit_SSD1306 which also requires Adafruit_GFX.h      - install both from Arduino IDE     **
** si5351 and WSPR encoding libraries are by Jason Milldrum                 - install both from Arduino IDE     **
** ESP32 - My multi band extension of this sketch uses the library "Sunset" and that requires a 32 bit FPU      **
** hence ESP32. ESP32 comes in 30 and 38 pin varieties. Tested with both varieties                              **
**                                                                                                              **
** si5351 square wave generator Clock0 is used as WSPR output.                                                  **
** si5351, DS3231 real time clock, and OLED SSD1306, conected via I2C bus on GPIO21 (SDA) and pin GPIO22 (SCL)  ** 
******************************************************************************************************************/
/*  
  WSPR signals are 6 Hz wide.   
 The WSPR software will decode 100 Hz either side of the Centre Frequency, which gives a total receive bandwidth
 of only 200 Hz.
 The following dial frequencies are in use for WSPR. We need to add 1500Hz to the dial frequency to get the transmit frequency:
 TX centre Frequency = USB Dial Frequency + 1.5 KHz
 Dial Frequency           TX Frequency
80m: 3.568600 MHz         3,570.100 kHz  +/- 100 Hz
40m: 7.038600 MHz         7,040.100 kHz  +/- 100 Hz
30m: 10.138700 MHz        10,140.200 kHz +/- 100 Hz
20m: 14.095600 MHz        14,097.100 kHz +/- 100 Hz 
17m: 18.104600 MHz        18,106.100 kHz +/- 100 Hz
15m: 21.094600 MHz        21,096.100 kHz +/- 100 Hz
12m: 24.924600 MHz        24,926.100 kHz +/- 100 Hz
10m: 28.124600 MHz        28,126.100 kHz +/- 100 Hz 

WSPR Time Slots - Must start on an EVEN minute so some "Number of slots" can't be used. For example, 4 slots would mean TX every 15 minutes at 0,15,30,45. 7 would mean every 11 minutes
There are 2 distinct periods when you can transmit, 0,4,8,12,16 etc minutes and 2,6,10,14,18 etc minutes. I designated thes Odd and Even in the table.
Recomended TX 20% of the time which gives 3 slots in each TX period.
1 slot is different because minutes goes from 0 to 59 and we have to choose so that (minute modulus mins between TX) = 0. Any value above 30 will do.

		                                        	Period	1	2	3	4	5	 6  7	   8	9	  10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30 	Odd	Even
Num Slots	Mins TX 	%TX	      Mins between TX       	0	2	4	6	8	10	12	14	16	18	20	22	24	26	28	30	32	34	36	38	40	42	44	46	48	50	52	54	56	58		
     1       2	    3.33	     anything > 30                                                                                                     X                    1    0                                                                                                  																						                                            						
     2	     4	    6.67	          30	              X													                        X														                                 	1	   1
     3	     6	    10.00	          20	              X									           	X										                     X									                    	3	   0
     5	    10	    16.67         	12	              X					    X						             X					           	X						            X						              5	   0
     6	    12	    20.00	          10	              X					X				            X					          X					           X					        X			                3    3 
     8	    16	    26.67	           8	              X				X				       X	 			       X	 			      X	 			       X	 			        X	 			         X	    	8    0
    10	    20	    33.33	           6	              X			X			  X			       X			     X		      X			      X			      X			      X			   X			     X    5	   5
    15	    30	    50.00	           4	              X		X   X	 	  X		     X	   	X		     X	 	   X		  X	    	X		   X	    	X		    X	 	     X		   X				15	 0


*/
/* Most boards have the built in LED connected to digital pin 13 and normally have a definition for LED_BUILTIN
However, I've recently come accros some boards that don't have LED_BUILTIN defined and the LED is connected to pin2. If you have
a board where LED_BUILTIN isn't defined the following code will define LED_BUILTIN = 2 and allow the program to compile and load.
This means that the LED will come on when WSPR is transmitting. If the led doesn't come on you'll need to try other values.
*/ 
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

/******************************[ Libraries ]***********************************************************************************/
#define DEBUG                  // Comment this line to supress debugging to the serial port
#include "Wire.h"              // arduino
#include <si5351.h>            // arduino
#include <JTEncode.h>          // arduino
#include <TimeLib.h>           //  https://github.com/PaulStoffregen/Time
#include <RTClib.h>            // arduino
#include <Adafruit_GFX.h>      // arduino
#include <Adafruit_SSD1306.h>  // arduino
#include <NTPtimeESP.h>        //  https://github.com/SensorsIot/NTPtimeESP  // you're only allowed to get the time every 4 seconds from an NTP server or you will be banned

/******************************[ WSPR ]********************************************/
char callsign[7] = "******";  //  USER CALLSIGN
char locator[5] = "****";     //  USER MAIDENHEAD GRID LOCATOR first 4 characters.
// txPower is determined by si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA) + any amplification you add
int txPower = 10;         // your actual TX power in dBm.
int timeSlots = 6;        // number of slots to transmit in each hour. Allowed values 1,2,3,5,6,8,10 and 15 corresponding to 3.33% of the time 6.67%,10%,16.67%,20%,26.67%,33.33%,50%
int randomChange = 75;    // 0 to 100.  a random value between -randomChange and +randomChange is applied to the TX frequency random(-100, 100)
#define TONE_SPACING 146  // ~1.46 Hz
#define WSPR_DELAY 683    // Delay value for WSPR
#define WSPR_CTC 10672    // CTC value for WSPR
#define SYMBOL_COUNT WSPR_SYMBOL_COUNT
JTEncode jtencode;                //create instance
uint8_t tx_buffer[SYMBOL_COUNT];  // create buffer to hold TX chars

/*****************************[ Wi-Fi ]*********************************************/
const char* ssid = "*****";              // SSID of your Wifi network
const char* password = "*******";        // Password of your wifi network
int failCount = 20;                      // maximun number of times to attempt to connect to wi-fi. Attempts are 500Ms appart
const char* WiFi_hostname = "ESP_WSPR";  // how it's reported on your router/hub
/******************************[ si5351 ]*******************************************************************************************/
int32_t cal_factor = 8100;  //Calibration factor obtained from Calibration arduino program in Examples. You must calbrate first
Si5351 si5351(0x60);        // si5351 instance. I've put I2C address in because sometimes it didn't seem to respond without it
/******************************[ OLED Display ]**************************************************************************************/
int SCREEN_WIDTH = 128;                                            // OLED display width, in pixels
int SCREEN_HEIGHT = 64;                                            // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // create an instance of Adafruit_SSD1306 called display
/************************************************* RTC ************************************************************************/
RTC_DS3231 rtc;
const char* weekDays[] = { "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat" };  // Used for the display only
/******************************[ NTP ]*****************************************************************************************/
#define DST_OFFSET 0                         // 1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment;  not tested by me but since I'll use UTC only it can stay at 0)
#define TIME_ZONE +0.0f                      // used in NTP time calculation. UTC time zone difference in regards to UTC (floating point number)
const char* NTP_Server = "uk.pool.ntp.org";  // pick an ntp server in your area
NTPtime NTPch(NTP_Server);
strDateTime NTPdateTime;  //strDateTime is declared in NTPtimeESP.h as a type - don't confuse with DateTime declared in RTClib
/*****************************[ Other Global Variables]*******************************/
#define BAUDRATE 115200  // Arduino serial monitor
// WSPR TX frequency = dial + 1500Hz Uncomment only 1, the one you are using
//const unsigned long freq =  28126100UL;  // 10Mtrs
//const unsigned long freq =  24926100UL;  // 12Mtrs
//const unsigned long freq =  21096100UL;  // 15Mtrs
const unsigned long freq = 18106100UL;  // 17Mtrs
//const unsigned long freq =  14097100UL;  // 20Mtrs
//const unsigned long freq =  10140200UL;  // 30Mtrs
//const unsigned long freq =   7040100UL;  // 40Mtrs
//const unsigned long freq =   3568750UL;  // 80Mtrs
float freqMHz;         //  the frequency to show on the OLED e.g. 18.106100
unsigned long txFreq;  // the actual frequency being transmitted = freq +- random(randomChange)

/***************************** [ initialiseDisplay ] *****************************************
***                             Initialise the OLED                                        ***
**********************************************************************************************/
void initialiseDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address found in I2C_Scanner.ino = 0x3C
#ifdef DEBUG
    Serial.println("SSD1306 allocation failed");  // if the display isn't found you can't write an error message on it
    Serial.println(" Program will continue");
#endif
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("OLED OK");
    display.display();  // you need display.display() or you don't update the display
#ifdef DEBUG
    Serial.println("initialiseDisplay finished");
#endif
  }
}

/**************************** [ initialiseWiFi ] ************************************
***                     Initialse and connect to Wi-Fi                            ***
*************************************************************************************/
void initialiseWiFi() {
  int attempts = 0;
  WiFi.mode(WIFI_OFF);  //Prevents reconnection issue (taking too long to connect)
  WiFi.mode(WIFI_STA);  //This line hides the viewing of ESP as wifi hotspot
  delay(1000);          // or you won't see the starting messages
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Connecting to Wi-Fi");
  display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {  // will be stuck in this loop till connected or timeout at about 10 seconds for failCount = 20
    attempts++;
    display.print("Wi-Fi attempts = ");
    display.println(attempts);
    display.display();
#ifdef DEBUG
    Serial.print("Attempting Wi-Fi connection. Number of attempts = ");
    Serial.println(attempts);
#endif
    if (attempts % 7 == 0) {
      display.clearDisplay();
      display.setCursor(0, 10);  // go back to the top of the screen
    }
    if (attempts > failCount) {
#ifdef DEBUG
      Serial.print("Failed to connect to Wi-Fi attempts = ");
      Serial.println(attempts);
      Serial.println("Are you sure you have the correct ssid and password?");
      Serial.print("ssid entered = ");
      Serial.print(ssid);
      Serial.print("  password entered = ");
      Serial.println(password);
      Serial.println("Program Halted");
#endif
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println(" Can't connect Wi-Fi");
      display.println("ssid and password OK?");
      display.print("ssid  ");
      display.println(ssid);
      display.print("pass ");
      display.println(password);
      display.println("Program Halted");
      display.display();
      while (1) {};  // program will stick here to let user read display
    }
    display.display();
    delay(500);
  }
}

/********************* [ updateRTC ] **************************************
*** Update RTC from NTP                                                 ***
*** The OLED will show the time continuously unless we are transmitting ***
*** During a TX cycle, I don't want to do anything apart from encode    ***
*** so the OLED clock will not update                                   ***
***************************************************************************/
void updateRTC() {
#ifdef DEBUG
  Serial.println();
  Serial.println("Updating RTC from NTP");
#endif
  do {
    NTPdateTime = NTPch.getNTPtime(TIME_ZONE, DST_OFFSET);  //1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment; ( not tested by me)
    delay(1);
  } while (!NTPdateTime.valid);  // keep trying till you get a valid time. rtc.adjust comes from the adafruit rtc lib
  rtc.adjust(DateTime(NTPdateTime.year, NTPdateTime.month, NTPdateTime.day, NTPdateTime.hour, NTPdateTime.minute, NTPdateTime.second));
#ifdef DEBUG
  Serial.println();
  Serial.println("RTC updated from NTP");
#endif
}

/***************  [ initialiseRTC ] **************************
*** Initialise the RTC and update from NTP server          ***
**************************************************************/
void initialiseRTC() {
  if (!rtc.begin()) {
#ifdef DEBUG
    Serial.println();
    Serial.println(" Couldn't find RTC.  Program will halt");
#endif
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Couldn't find RTC. Program will halt ");
    display.display();
    while (1) delay(10);  // stay here until rtc found - if it's not found you're here forever
  }
  configTime(0, 0, NTP_Server);
#ifdef DEBUG
  Serial.println("initialise RTC finished");
#endif
  updateRTC();  // now set it to correct time
}

/**************************  [ initialiseSI5351 ] ***********************************
***                Initialse the si5351 square wave generator                     ***
*************************************************************************************/
void initialiseSI5351() {
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  txFreq = freq + random(-randomChange, randomChange);
  freqMHz = txFreq / 1000000.0;                  // used to give the user a meaningful displayed frequency. If you divide with an integer, it only returns the quotient as an integer
  si5351.set_freq((txFreq * 100), SI5351_CLK0);  // Clock 0 used to tx WSPR
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  // If you change the drive strength to another value you will need to change the txPower value at line 76
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);  // 2,4,6,8mA  2 mA roughly corresponds to 3 dBm output,8 mA is approximately 10 dBm
  si5351.set_clock_pwr(SI5351_CLK0, 0);                  // Disable the clock initially
#ifdef DEBUG
  Serial.println("initialiseSI5351 finished");
#endif
}

/************************************ [ serialShowTime ] *************************************
***  Only used in debug. Show date and time in serial monitor                              ***
**********************************************************************************************/
void serialShowTime(DateTime now) {
  Serial.print(weekDays[now.dayOfTheWeek()]);
  Serial.print("  ");
  serialPadZero(now.day());
  Serial.print("/");
  serialPadZero(now.month());
  Serial.print("/");
  serialPadZero(now.year());
  Serial.print("  ");
  serialPadZero(now.hour());
  Serial.print(":");
  serialPadZero(now.minute());
  Serial.print(":");
  serialPadZero(now.second());
}

/************************************ [ mainScreen ] *****************************************
***  This is the data I want to see all the time on the OLED                               ***
***  1st line ssid, 2nd line I.P address, 3rd line frequency, 4th date, 5th current time   ***
**********************************************************************************************/
void mainScreen() {
  static int lastSecond = 0;
  DateTime now = rtc.now();
  if (!(lastSecond == now.second())) {  // only display the main screen if the time has changed otherwise screen flashes
    display.clearDisplay();             // connected at this point
    display.setCursor(0, 10);
    display.println(ssid);
    display.println(WiFi.localIP());
    display.print(freqMHz, 6);
    display.println(" MHz");
    showDay(now);
    display.print("  ");
    showDate(now);
    display.println();
    showTime(now);
    display.println();
    lastSecond = now.second();
    display.display();
#ifdef DEBUG
    Serial.println();
    Serial.print(ssid);
    Serial.print("   I.P. = ");
    Serial.println(WiFi.localIP());
    Serial.print("Frequency = ");
    Serial.print(freqMHz, 6);
    Serial.println(" MHz");
    serialShowTime(now);
    Serial.println();
    Serial.print("Callsign ");
    Serial.print(callsign);
    Serial.print("   ");
    Serial.print("Locator ");
    Serial.print(locator);
    Serial.print("  Power ");
    Serial.print(txPower);
    Serial.println("dBm");
#endif
  }
}

/********************************* [ showDay ] ***************************************************
***          Display the Day on the OLED                                                       ***
***  since we only use the NTP server to update the RTC I've decided only to display RTC data  ***
**************************************************************************************************/
void showDay(DateTime now) {
  display.print(weekDays[now.dayOfTheWeek()]);
  display.display();
}

/******************************** [ showDate ] ***************************************************
***          Display the Date on the OLED                                                      ***
***  since we only use the NTP server to update the RTC I've decided only to display RTC data  ***
**************************************************************************************************/
void showDate(DateTime now) {
  displayPadZero(now.day());
  display.print("/");
  displayPadZero(now.month());
  display.print("/");
  displayPadZero(now.year());
  display.display();
}

/********************************** [ showTime ] *************************************************
***          Display the Time on the OLED                                                      ***
***  since we only use the NTP server to update the RTC I've decided only to display RTC data  ***
**************************************************************************************************/
void showTime(DateTime now) {
  displayPadZero(now.hour());
  display.print(":");
  displayPadZero(now.minute());
  display.print(":");
  displayPadZero(now.second());
  display.display();
}

/************************** [ serialPadZero ] ******************************************
*** Print a "0" in front of a single digit number in the Serial Monitor Output       ***
****************************************************************************************/
void serialPadZero(int aNumber) {
  if (aNumber < 10) {
    Serial.print("0");
    Serial.print(aNumber);
  } else {
    Serial.print(aNumber);
  }
}

/**********************************  [ displayPadZero ]  ************************************
*** Print a "0" in front of a single digit number on the OLED display                     ***
*********************************************************************************************/
void displayPadZero(int aNumber) {
  if (aNumber < 10) {
    display.print("0");
    display.print(aNumber);
  } else {
    display.print(aNumber);
  }
  display.display();  // if you dont do display.display() it doesn't display. Very annoying
}

/********************* [ encode] *********************************
***                WSPR encode and transmit                    ***
*** When in this function the time on the OLED will not update ***
******************************************************************/
void encode() {
  uint8_t i;
#ifdef DEBUG
  Serial.println("*** Encode In ***");
#endif
  digitalWrite(LED_BUILTIN, HIGH);  // tell user we are now transmitting
  jtencode.wspr_encode(callsign, locator, txPower, tx_buffer);
  for (i = 0; i < SYMBOL_COUNT; i++) {
    si5351.set_freq((txFreq * 100) + (tx_buffer[i] * TONE_SPACING), SI5351_CLK0);
    delay(WSPR_DELAY);
  }
  // si5351.set_clock_pwr(SI5351_CLK0, 0);  // Finished TX so Turn off the output
  digitalWrite(LED_BUILTIN, LOW);  // tx off
#ifdef DEBUG
  Serial.println("*** Encode Out ***");
#endif
  mainScreen();
}

/***************************** [ txDelay ] ******************************************
***          txDelay() Return number of minutes between transmissions             ***
*************************************************************************************/
int txDelay(int numSlots) {
  int numMinutes = 10;  // 20% TX time, default value
  switch (numSlots) {
    case 1:             // 60 min cycle
      numMinutes = 38;  // any even number above 30 because usage is      now.minute() % numMinutes == 0
      break;
    case 2:
      numMinutes = 30;
      break;
    case 3:
      numMinutes = 20;
      break;
    case 5:
      numMinutes = 12;
      break;
    case 6:
      numMinutes = 10;
      break;
    case 8:
      numMinutes = 8;
      break;
    case 10:
      numMinutes = 6;
      break;
    case 15:
      numMinutes = 4;
      break;
    default:
      numMinutes = 10;
      break;
  }
  return numMinutes;
}

/***************************** [ txOn ] ***********************************************
*** Neither switching TX on a couple of seconds early or having clock1 running at   ***
*** 150MHz when clock0 wasn't transmitting made any difference to my drift problem. ***
*** My drift problems were solved by using another si5351                           ***
**************************************************************************************/
void txOn() {
  txFreq = freq + random(-randomChange, randomChange);
  freqMHz = txFreq / 1000000.0;
  si5351.set_freq((txFreq * 100), SI5351_CLK0);
  si5351.set_clock_pwr(SI5351_CLK0, 1);  // switch on clock0
}

/************* [ txOff ] ***************
*** Switch clock0 off                ***
****************************************/
void txOff() {
  si5351.set_clock_pwr(SI5351_CLK0, 0);  // stop transmitting. Switch off clock 0
}
//**************************************[ SETUP FUNCTION ]*************************************************
void setup() {
  Serial.begin(BAUDRATE);
  pinMode(LED_BUILTIN, OUTPUT);  // used as indicator that we are transmitting.
  initialiseDisplay();
  initialiseWiFi();  // If we don't get an internet connection we'll be stuck in the trying to connect loop
  initialiseRTC();   // needs wi-fi. RTC updated from NTP in this function
  delay(1000);
  initialiseSI5351();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  mainScreen();
  display.display();
  randomSeed(analogRead(0));  // get an arbitrary starting seed
#ifdef DEBUG
  if ((randomChange > 100) || (randomChange < 0)) {
    Serial.print("randomChange = ");
    Serial.print(randomChange);
    Serial.print("   ");
    Serial.println(" It must be >= 0 and <= 100");
    Serial.println("*******    Program Halted    *****************");
  } else {
    Serial.println("Leaving Setup()");
  }
#endif
  if ((randomChange > 100) || (randomChange < 0)) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.print("randomChange= ");
    display.println(randomChange);
    display.println("It must be ");
    display.println("  >= 0 and <= 100");
    display.println();
    display.println("Program Halted");
    display.display();
    while (1) {};  // stay here for ever
  }
}
//*****************************************[ LOOP FUNCTION ]******************************************************
void loop() {
  DateTime now = rtc.now();
  mainScreen();
  if ((now.minute() % 32 == 0) && (now.second() % 37 == 0)) {  // Update RTC every hour
    updateRTC();
   }

  if ((now.minute() % txDelay(timeSlots) == 0) && (now.second() == 0)) {  //start encoding at start of even minute
    digitalWrite(LED_BUILTIN, HIGH);
    txOn();
#ifdef DEBUG
    Serial.println("************************************************* Loop TX ON *******");
#endif
    display.println("Loop TX ON");
    display.display();
    encode();  // transmit the codes
    txOff();
#ifdef DEBUG
    Serial.println("************************************************* Loop TX OFF *******");
#endif
    digitalWrite(LED_BUILTIN, LOW);
  }
}
//*************************************[ END OF PROGRAM ]********************************************************
