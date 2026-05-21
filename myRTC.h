/*
  SD card datalogger on Adafruit Data Logger Shield Rev C, with CR1220 3V battery
  I am using just its Real Time Clock (PCF8523) for this project.
*/

#include <RTClib.h>

constexpr byte chip_select_pin = 10;  // Adafruit SD shields: pin 10
RTC_PCF8523 rtc;                      // Real Time Clock object

void setup_logger() {
  pinMode(chip_select_pin, OUTPUT); // must be set to output, even if it isn't used.

  Serial.print(F("RTC... "));
  
  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1) delay(10);
  }
  Serial.println(F("OK"));

  if (! rtc.initialized() || rtc.lostPower()) {
    // When time needs to be set on a new device, or after a power loss,
    // set the RTC to the date & time of PC this sketch was compiled on.
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("RTC updated"));
  }
  
  // When the RTC was stopped and stays connected to the battery, it has
  // to be restarted by clearing the STOP bit. Let's do this to ensure
  // the RTC is running.
  rtc.start();
}

byte remaining_minutes_to_break(const DateTime now) {
  static constexpr struct { const byte hour, minute; } breaks[] = {
     {9,60}, {9,50}, {10,45}, {11,40}, {12,35}, {13,25}, {14,10}  };
  constexpr byte breaks_length = sizeof(breaks) / sizeof(breaks[0]);

  int rem_min{0};
  int i{0};
  for (; i < breaks_length; ++i) {
    if(breaks[i].hour == now.hour() and now.minute() < breaks[i].minute) {
      rem_min = breaks[i].minute - now.minute();
      break;
    }
    else if (breaks[i].hour == now.hour() + 1) {
      rem_min = 60 - now.minute() + breaks[i].minute;
      break;
    }
  }
  // Serial.println((String)"[" + i + "] " + now.hour() + ':' + now.minute() 
  //   + " -> " + breaks[i].hour + ':' + breaks[i].minute + " = " + rem_min);
  return rem_min;
}