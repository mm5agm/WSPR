These programs/sketches are from the article "A beginners guide to the Arduino IDE culminating in a single band WSPR beacon" in Practical Wireless, a UK magazine. Publication date to be advised.
Practical Wireless hold the copyright to the printed article and with their permission it will be put here as well.

![Fig 1  Block Diagram](https://github.com/mm5agm/WSPR/assets/26571503/d4f9af83-ccb1-496f-a32a-023caceaa8cf)

If you download the sketches in order you will be able to see how each component works and end up with a single band WSPR beacon. You must be a licenced radio amateur to use the WSPR beacon. If you're new to the Arduino you may want to go through the sketches in order otherwise just load the last one. The order is
1) Add ESP32 - Get time from NTP server - NTP_Basic.ino
4) Add real time clock.  Scan I2C to find devices - I2C_Scanner.ino
7) Update RTC from NTP - NTP_Time_With_RTC.ino
8) Add OLED display, Scan I2C to get OLED address
9) Display information on OLED - NTP_Time_RTC_OLED.ino
10) Add si5351 square wave generator - calibrate using Examples/Etherkit Si5351/si5351_calibration.ino in the Arduino IDE
11) Add low pass filter for the band you are using.
12) Load WSPR_Single_Band (not on github yet )
    
