/*
  WallClock project.

  It displays the Date-Time and remaining period minutes before the next break.
  Breaks correspond to the Greek Gymnasium timetable schedule.

  Hardware list:
  a) LED Matrix Panel 64x32p/16 by WaveShare, wired via HUB75
  b) Arduino Mega RTK or similar
  c) Any shield with a Real Time Clock (I used Adafruit Data Logger Shield Rev C)

  Software list:
  C++/17
  Adafruit RGB Matrix Panel library
  A real time clock (RTC) library
*/

#include "myRTC.h"
#include <RGBmatrixPanel.h>

#define R1  24
#define G1  25
#define B1  26
#define R2  27
#define G2  28
#define B2  29

#define A   A0
#define B   A1
#define C   A2
#define D   A3

#define CLK 11 //D11 is used by SD Card SPI MOSI, can move it to Pins 12, 13, 50, 51, 52, or 53
#define OE   9 // You can move it to any unused digital pin.
#define LAT  8 //D10 is used by SD Card Chip Select (CS), can move it to any unused digital pin.

#define horiz_LEDs 64
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, horiz_LEDs);

void setup() {
  Serial.begin(9600);
  Serial.println("\nSerial OK");

  setup_logger();

  setup_registers();
  matrix.begin();
  delay(100);
  // matrix.setFont(NULL);
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves
  Serial.println("Screen OK");
}

void loop() {
  const DateTime now = rtc.now();
  static const auto black = matrix.Color444(0, 0, 0);

  if (now.second() > 0) { // if only seconds change
    // Clear only the two digits of seconds
    matrix.fillRect(44, 9, 11, 7, black);
  } else {
    // Clear all the time digits and remaining minutes to break
    matrix.fillRect(9, 9, 49, 23, black);
  }
  
  matrix.setTextSize(1);     // size 1 == 8 pixels high
  matrix.setCursor(3, 1);    // start at top left, with 8 pixel of spacing
  matrix.setTextColor(matrix.Color444(0, 0, 255));
  constexpr char date_fmt_buff[] = "DDD DD MMM";
  matrix.print(now.toString(date_fmt_buff));

  matrix.setCursor(8, 9);
  matrix.setTextColor(matrix.Color444(0, 255, 0));
  constexpr char time_fmt_buff[] = "hh:mm:ss";
  matrix.print(now.toString(time_fmt_buff));

  matrix.setTextSize(2);
  matrix.setCursor(18, 17);
  matrix.setTextColor(matrix.Color444(0, 255, 255));
  matrix.print((String)remaining_minutes_to_break(now) + '\'');

  // Serial.println(time_stamp);
  delay(1000);
}

constexpr byte all_pins[] = {R1, G1, B1, R2, G2, B2, CLK, OE, LAT};
constexpr byte all_pins_length = sizeof(all_pins) / sizeof(all_pins[0]);

void setup_registers() {
  for (byte pin = 0; pin < all_pins_length; pin++) {
    pinMode(all_pins[pin], OUTPUT);
  }
  digitalWrite(OE, HIGH);
  digitalWrite(LAT, LOW);
  digitalWrite(CLK, LOW);

  constexpr byte control_reg_12[16] = {0, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1};
  initialize_register(control_reg_12, 12);

  constexpr byte control_reg_13[16] = {0, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0};
  initialize_register(control_reg_13, 13);
}

// WaveShare LED Matrix Panel 64x32p/16 needs a special way to initialize!
// Chips: driver SM16208SJ, multiplexer RUC7258D,

void initialize_register(const byte control_reg[], const byte led_reg) {
  // Send Data to control register 12 or 13
  for (byte led{0}; led < horiz_LEDs; ++led)   {
    write_RGB_pins(LOW);

    if (control_reg[led % 16] == 1) {
      write_RGB_pins(HIGH);
    }
    if (led > horiz_LEDs - led_reg) {
      digitalWrite(LAT, HIGH);
    } else {
      digitalWrite(LAT, LOW);
    }
    digitalWrite(CLK, HIGH);
    delayMicroseconds(2);
    digitalWrite(CLK, LOW);
  }
  digitalWrite(LAT, LOW);
  digitalWrite(CLK, LOW);
}

void write_RGB_pins(const byte level) {
  for (byte RGB_pin{0}; RGB_pin < 2*3; ++RGB_pin) {
    digitalWrite(all_pins[RGB_pin], level);   
  }
}
