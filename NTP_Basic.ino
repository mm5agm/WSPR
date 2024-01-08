/* 
A simple test program to get date and time from an NTP server
  NTP library is https://github.com/SensorsIot/NTPtimeESP 
  you're allowed to get the time every 4 seconds but 1 or twice a day to update a real time clock 
  should be sufficient
*/
#include <NTPtimeESP.h>
//NTP Server:
const char* NTP_Server = "uk.pool.ntp.org";  // pick an ntp server in your area
NTPtime NTPch(NTP_Server);
strDateTime NTPdateTime;           //strDateTime is declared in NTPtimeESP.h as a type
const char* ssid = "*****";        // SSID of your Wifi network
const char* password = "*******";  // Password of your wifi network
int failCount = 60;                // maximun number of times to attempt to connect to wi-fi. Attempts are 500Ms appart
int BAUDRATE = 9600;
#define DST_OFFSET 0     //  1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment;
#define TIME_ZONE +0.0f  // used in NTP time calculation. UTC time zone difference in regards to UTC (floating point number)

void initialiseWiFi() {
  int attempts = 0;
  WiFi.mode(WIFI_OFF);  //Prevents reconnection issue (taking too long to connect)
  WiFi.mode(WIFI_STA);  //This line hides the viewing of ESP as wifi hotspot
  Serial.print("Connecting to Wi-Fi   ");
  Serial.println(ssid);
  Serial.println(password);
  Serial.print("- Connecting");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
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
  // If we get to here weve connected to wifi
  Serial.println();
  Serial.print(F("- Connected to: "));
  Serial.println(ssid);
  Serial.print(F("- IP address: "));
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(BAUDRATE);
  Serial.println("Starting Up");
  initialiseWiFi();  // If we don't get an internet connection we'll be stuck in the trying to connect loop
}
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
void loop() {
  do {
    NTPdateTime = NTPch.getNTPtime(TIME_ZONE, DST_OFFSET); 
    delay(1);
  } while (!NTPdateTime.valid);  // keep trying till you get a valid time
  if (NTPdateTime.valid) {
    showDate(NTPdateTime);
  }
  delay(10000);  // the keepers of the NTP servers will ban you if you request the time more often than once every 4 seconds
}
