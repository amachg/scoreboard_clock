/*
  This started as a ScoareBoard project but it ended up as a 

  WallClock project.

  It displays the Date-Time and remaining period minutes before the next break.
  Breaks correspond to the Greek Gymnasium timetable schedule.

  Hardware list:
  LED Matrix Panel 64x32p wired via HUB75 to an
  Arduino Mega RTK and a 
  Shield with a Real Time Clock.
*/

#include "RGBmatrixPanel.h"
#include "Data_logger.h"  // 

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

constexpr int RGB_pins[6] = {R1, G1, B1, R2, G2, B2};
// const byte all_pins[13] = {R1, G1, B1, R2, G2, B2, CLK, OE, LAT};

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64);

void setup() {
  Serial.begin(9600);
  Serial.println("Serial OK");

  setup_registers();
  matrix.begin();
  delay(100);
  // matrix.setFont(NULL);
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves
  Serial.println("Screen OK");

  setup_logger();
}

void loop() {
  // fetch and log the time
  const DateTime now = rtc.now();  
  char date_fmt_buff[] = "DDD DD MMM";
  const String date_stamp = now.toString(date_fmt_buff);
  char time_fmt_buff[] = "hh:mm:ss";
  const String time_stamp = now.toString(time_fmt_buff);
  // Serial.println(time_stamp);
  
  matrix.setTextSize(1);     // size 1 == 8 pixels high
  matrix.setCursor(3, 1);    // start at top left, with 8 pixel of spacing
  matrix.setTextColor(matrix.Color444(0, 0, 255));
  matrix.print(date_stamp);

  matrix.setCursor(8, 9);
  matrix.setTextColor(matrix.Color444(0, 255, 0));
  matrix.print(time_stamp);

  matrix.setTextSize(2);
  matrix.setCursor(18, 17);
  matrix.setTextColor(matrix.Color444(0, 255, 255));
  matrix.print((String)remaining_minutes(now) + '\'');

  delay(5000);
  matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.Color444(0, 0, 0));
}

int remaining_minutes(const DateTime now) {
  const struct { int hour, minute; } breaks[] 
    = { {9,60}, {9,50}, {10,45}, {11,40}, {12,35}, {13,25}, {14,10} };

  int rem_min = -1;
  for (int i = 0; i < 7; ++i) {
    if(breaks[i].hour == now.hour() and now.minute() < breaks[i].minute)
      rem_min = breaks[i].minute - now.minute();
    else if (breaks[i].hour == now.hour() + 1)
      rem_min = 60 - now.minute() + breaks[i].minute;

    // Serial.println((String)"[" + i + ']' + now.hour() + ':' + now.minute() 
    //   + " -> " + breaks[i].hour + ':' + breaks[i].minute + " = " + rem_min);
  }
  return rem_min;
}

void setup_registers() {
  for (int pin = 0; pin < 6; pin++)
    pinMode(RGB_pins[pin], OUTPUT);

  pinMode(CLK, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(LAT, OUTPUT);

  digitalWrite(OE, HIGH);
  digitalWrite(LAT, LOW);
  digitalWrite(CLK, LOW);
  constexpr int control12[16] = {0, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1};
  initialize_register(control12, 12);
  constexpr int control13[16] = {0, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0};
  initialize_register(control13, 13);
}

void initialize_register(const int control_register[], const int led_reg) {
  // Send Data to control register 12 or 13
  int MaxLed = 64;
  for (int led = 0; led < MaxLed; led++)   {
    write_RGB_pins(LOW);

    if (control_register[led % 16] == 1)
      write_RGB_pins(HIGH);

    if (led > MaxLed - led_reg)
      digitalWrite(LAT, HIGH);
    else
      digitalWrite(LAT, LOW);

    digitalWrite(CLK, HIGH);
    delayMicroseconds(2);
    digitalWrite(CLK, LOW);
  }
  digitalWrite(LAT, LOW);
  digitalWrite(CLK, LOW);
}

void write_RGB_pins(const byte level) {
    for (int p = 0; p < 6; p++)
      digitalWrite(RGB_pins[p], level);   
}
