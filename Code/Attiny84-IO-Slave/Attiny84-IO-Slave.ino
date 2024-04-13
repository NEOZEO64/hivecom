/* Attiny84 IO Chip Code
This code ignores calibrating and just passes on the direct weight cell data over I2C:

0. ATtiny & HX711 chips get powered by ESP32 GPIO pin
1. Initialize
2. Collect weight data (read average over 10 samples)
3. Wait for I2C request
4. ATtiny & HX711 chips get turned off as ESP32 GPIO pin turnes low
(But as long as the ATtiny is turned on it refreshes its sensor data registers every 100ms)
*/

#define __AVR_ATtiny84__
#define ARDUINO_ARCH_AVR
#define F_CPU 20000000  // clock speed

#include "HX711.h"
#include "TinyWireS.h" // library emulating I2C on ATtiny84A

union value { byte b[4]; long l; }; // multiple format data storing
volatile value vals[4] = {-1, -1, -1, -1}; // value 0-3: weight cell 1-4

const int ledPin = 5;

HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;

void requestI2C() {
  // Send 16 individual bytes where every 4-byte-pack represents one long which holds weight cell value
  for (int i=0; i<16; i++) { TinyWireS.write(vals[i/4].b[i%4]); }
  blink(1,20);
}

// General blink tool for debugging purposes
void blink(int count, int del) {
  for (int i = 0; i < count; i++) {
    digitalWrite(ledPin, true);
    delay(del);
    digitalWrite(ledPin, false);
    delay(del);
  }
  delay(100);
}

void setup() {
  //initialize wire / I2C protocol
  TinyWireS.begin(0x60); //0x60 is slave address

  //initialize debugging led
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, false);

  // initialize weight cells connected to specific ATtiny pins
  scale1.begin(10, 9); // data pin, clock pin
  scale2.begin(8, 7);
  scale3.begin(0, 1);
  scale4.begin(2, 3);

  // weight for cells (experience: gets ignored!)
  while (!scale1.is_ready()) { blink(1, 300); scale1.wait_ready(10); }
  while (!scale2.is_ready()) { blink(2, 300); scale2.wait_ready(10); }
  while (!scale3.is_ready()) { blink(3, 300); scale3.wait_ready(10); }
  while (!scale4.is_ready()) { blink(4, 300); scale4.wait_ready(10); }

  // prepare for I2C data request comming from ESP32
  TinyWireS.onRequest(requestI2C);
}

// just needs do be there
void loop() {
  TinyWireS_stop_check();

  delay(100);

  vals[0].l = scale1.read_average(10); // direct, uncalibrated weight values
  vals[1].l = scale2.read_average(10);
  vals[2].l = scale3.read_average(10);
  vals[3].l = scale4.read_average(10); 
}