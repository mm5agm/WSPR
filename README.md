These programs/sketches are from the article "A beginners guide to the Arduino IDE culminating in a single band WSPR beacon" in Practical Wireless, a UK magazine. Publication date is the May issue of PW (out early April) - it will probably run to two, maybe three articles.
Practical Wireless hold the copyright to the printed article and with their permission it will be printed here after publication.

![Fig 1  Block Diagram](https://github.com/mm5agm/WSPR/assets/26571503/d4f9af83-ccb1-496f-a32a-023caceaa8cf)

If you run the sketches in order you will be able to see how each component works and end up with a single band WSPR beacon. You must be a licenced radio amateur to use the WSPR beacon. If you're new to the Arduino you may want to go through the sketches in order otherwise just load the last one. I used Arduino Version 2.2.1
1) In the Arduino IDE go to “File/Preferences” and fill in the “Additional boards manager URLs” with https://espressif.github.io/arduino-esp32/package_esp32_index.json
2) Add ESP32 - Get time from NTP server                             - NTP_Basic.ino Change NTP_Server to one near you
3) Add real time clock.  Scan I2C to find devices                   - I2C_Scanner.ino
4) Update RTC from NTP                                              - NTP_Time_With_RTC.ino
5) Add OLED display, Scan I2C to get OLED address                   - I didn't need to use the OLED address
6) Display information on OLED                                      - NTP_Time_RTC_OLED.ino
7) Add si5351 square wave generator - calibrate using Examples/Etherkit Si5351/si5351_calibration.ino in the Arduino IDE. I changed target_freq on line 32 from 1000000000ULL to 10140200000ULL and used WSJT-X in WSPR mode on 30mtrs to see where my signal was.
8) Add low pass filter for the band you are using.
9) Load WSPR_Single_Band. Leave randomChange at 0 until you're sure that your TX is within the WSPR range. My drift problem was caused by a faulty si5351 so I deleted code that started TX 2 seconds early and that had clock1 at 150MHz when not transmitting WSPR.
    
