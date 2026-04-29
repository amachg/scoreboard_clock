/*
  SD card datalogger on Adafruit Data Logger Shield Rev C, with CR1220 3V battery
  I am using just its Real Time Clock (PCF8523) for this project
*/

#include <Wire.h>
#include "RTClib.h"

constexpr byte chip_select_pin = 10;  // Adafruit SD shields: pin 10
RTC_PCF8523 rtc;                      // Real Time Clock object

void setup_logger() {
  pinMode(chip_select_pin, OUTPUT); // must be set to output, even if it isn't used.

  Serial.print(F("RTC... "));
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1) ;
  }
  Serial.println(F("OK"));

  if (! rtc.initialized() || rtc.lostPower()) {
    // When time needs to be set on a new device, or after a power loss,
    // set the RTC to the date & time of PC this sketch was compiled on.
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("RTC updated"));
  }
}