/* HIVE COM Main uploading program (should run on ESP32 inside hive scale)

This should run on the weight & sensor system
Program workflow:
1. Initialize System
2. If debug button is pressed on startup:
    a. Show sensor values in a loop
    b. If debug button pressed again: change upload interval!
3. Else: start normal upload program
  a. Get sensor values
  b. Upload sensor values
  c. Go to sleep for specified time
  d. Then restart
*/

// weight cell calibration settings
//const long WEIGHT_OFFSETS[4] = {108015, 91354, 41291, 147069}; // with small wooden plate
const long WEIGHT_OFFSETS[4] = {83400, 282813, 24384, 323675}; // connected with big board 
const float WEIGHT_SCALE_FACTORS[4] = {1168659 / 10400, 1165328 / 10400, 1126508 / 10400, 1185686 / 10400}; 
const float WEIGHT_SCALE_FACTOR_ALL = 21091.14/20800;  // for all weight cells


#include <Wire.h> // for I2C communication with ATtiny as weight cell manager
#include <Adafruit_GFX.h> // for OLED display
#include <Adafruit_SSD1306.h>

#include <OneWire.h> // for DS18B20 waterproof temperature sensor
#include <DallasTemperature.h> 

#include <DHT.h> // for temperature & humidity sensor in Uploading Box

#include "hivecomlogo.h"
#include "loraTools.h" // for Lora communication
#include <EEPROM.h>



// OLED screeen settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DS18B20 temperature sensor settings (inside beehive)
OneWire oneWire(25);
DallasTemperature ds18b20_sensor(&oneWire);

// DHT22 temperature / humidity sensor (outside beehive)
DHT dht22_sensor(4, DHT22); //dhtpin / dhttype

union value { byte b[4]; long l; }; // multiple format data storing
volatile value vals[4] = {0,0,0,0}; // value 0-3: weight cell 1-4 raw, direct, uncalibrated
volatile float weights[5] = {-101, -101, -101, -101, -101}; // value 0: weight sum calibrated in grams, value 1-4: weight cell 1-4 calibrated in grams

const uint8_t ATTINY_ADDRESS = 0x60;
const uint8_t NUM_BYTES = 16; //4 long values for each weight cell
const uint8_t ATTINY_POWER_PIN = 12; // Attiny & HX711 weight cell amplifiers get power from ESP32 GPIO pin
const uint8_t DEBUG_BUTTON_PIN = 34;

uint8_t measurement_interval_index = 1;

const long measurement_intervals[] = { // in minutes
  1,
  5,
  20,
  60,
  120,
  360
};


#define ATTINY_I2C_ERROR 1
#define OK 0

void showLogo() {
  display.clearDisplay();
  display.drawBitmap(0, 0, hivecomlogo, 128, 64, 1); // offset the video pointer 16*64 bytes to get to the right image

  int x = 76;
  int y = 48;

  for (int t = 0; t < 5; t += 1) {
    for (int xOff = 0; xOff < 4; xOff ++) {
      display.drawRect(x,y,24,2, SSD1306_BLACK);
      for (int i = 0; i < 6; i += 1) {
        display.drawRect(x+ i*4 + xOff,48,2,2, SSD1306_WHITE);
      }
      display.display();
      delay(100);
    }
  }
}


int getWeight() {
  Wire.requestFrom(ATTINY_ADDRESS, NUM_BYTES);
  if (Wire.available() != NUM_BYTES) { for (int i=0; i<5; i++) { weights[i] = -1;} return ATTINY_I2C_ERROR; } // invalidate all weight values if faulty I2C connect
  else {
    for (int i=0; i<NUM_BYTES; i++) { vals[i/4].b[i%4] = Wire.read();} // collect all bytes & convert it to 4 long values
    
    weights[0] = 0;
    for (int i=0; i<4; i++) {
      weights[i+1] = ((float)(vals[i].l-WEIGHT_OFFSETS[i]))/WEIGHT_SCALE_FACTORS[i];
      weights[0] += weights[i+1];
    }
    weights[0] /= WEIGHT_SCALE_FACTOR_ALL; // calculate calibrated sum
  }
  return OK;
}

float getBatteryPercentage() {
  // reads voltage of battery and returns a percentage value (roughly approximated over two linear intervals)
  // voltage 3.6: 0%, turn off!
  // voltage 4.2: 100%
  // voltage >4.2: 150% (should be interpreted that it's charged)
  float voltage = analogRead(0)*2*3.3/4096;
  if (voltage > 4.3) {return 120;} // return 120 for sign to be charged

  // approximated voltage discharging profile on: https://learn.adafruit.com/li-ion-and-lipoly-batteries/voltages
  if (3.8 < voltage && voltage <= 4.3 ) { // approximate voltage to percentage in two linear intervals
    return 60 + 40 * (voltage-3.8)/0.5; 
  } else if (3.6 < voltage && voltage <= 3.8) {
    return 0 + 60 * (voltage-3.6)/0.2;
  } else {
    return 0;
  }
}

void noBattery() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("HIVE COM");
  display.drawFastHLine(0, 10, 127,WHITE);
  display.setCursor(0,12);
  display.print("NO BATTERY,\nGOING TO SLEEP\nUNTIL CHARGING AGAIN");
  display.display();

  delay(2000);
  display.clearDisplay();
  digitalWrite(ATTINY_POWER_PIN, false);

  // sleep forever! (until reset)
  esp_sleep_enable_timer_wakeup(measurement_intervals[measurement_interval_index]*1000*1000*60 * 99999);
  esp_deep_sleep_start();
}


void debugAction() { //Own, separate program
  Serial.println("Started debug mode");
  Serial.println("Starting, wait for ATtiny84 weight cell manager");
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("HIVE COM DEBUG");
  display.drawFastHLine(0, 10, 127,WHITE);
  display.setCursor(0,12);
  display.print("Starting debug mode");
  display.display();
  
  delay(4000);

  long then = millis();
  while (true) {
    if (digitalRead(DEBUG_BUTTON_PIN) == false) { // if debug button is pressed
      measurement_interval_index += 1;
      measurement_interval_index %= 6;
      EEPROM.writeUChar(0, measurement_interval_index);
      EEPROM.commit();

      display.clearDisplay();
      display.setCursor(0,0);
      display.print("HIVE COM DEBUG");
      display.drawFastHLine(0, 10, 127,WHITE);
      display.setCursor(0,12);
      display.print("Changed measurement\ninterval to:\n");
      display.print(measurement_intervals[measurement_interval_index]);
      display.print("min");
      display.display();
      delay(2000);
      then = 0;
    }



    long now = millis();
    if (now-then > 300) {
        // Collect sensor data
      float tempOut = dht22_sensor.readTemperature(); // DHT22 temperature & humidity sensor (outside)
      float humOut = dht22_sensor.readHumidity();
      ds18b20_sensor.requestTemperatures(); // DS18B20 waterproof temperature sensor (inside)
      float tempIn = ds18b20_sensor.getTempCByIndex(0);

      // Battery Percentage
      float batteryPercentage = getBatteryPercentage();
      if (batteryPercentage == 0) { noBattery(); }

      // Weight values
      int status = getWeight();
      String sensors, raw, offs, sca, sca2, cmp;

      if (status == OK) { // Get raw weight values from ATtiny and calculate real weight values in kilograms
        raw = "RAW: "; // print raw values
        for (int i = 0; i < 4; i++) {raw += String(vals[i].l); raw += ",\t\t";}
        
        offs = "OFF: "; // print raw, but offsetted values
        for (int i = 0; i < 4; i++) {offs += String(vals[i].l- WEIGHT_OFFSETS[i]); offs += ",\t\t";}

        sca = "SCA: "; // print scaled & offsetted values (for serial monitor)
        for (int i = 0; i < 4; i++) {sca += String(weights[i+1],0); sca += "g,\t\t";}
        sca2 = ""; // print scaled & offsetted values (for small oled screen)
        for (int i = 0; i < 4; i++) {sca2 += String(weights[i+1],0); sca2 += "g,";}


        cmp = "COMPLETE: ";
        cmp += weights[0];
        cmp += "g";
      } else {
        cmp = "ATtiny I2C Error";
      }

      // Show sensor data on OLED screen
      String batMsg = String("Bat:") + String(batteryPercentage,0) + "%";
      if (batteryPercentage == 120) {
        batMsg += " (charging)";
      }
      String dhtMsg = String("Out:") + String(tempOut,2) + "C " + String(humOut) + "%";
      String dsMsg = String("In: ") + String(tempIn, 2) + "C";

      Serial.println(batMsg + "\t" + dhtMsg + "\t" + dsMsg);

      if (status == OK) {
        Serial.println(raw);  
        Serial.println(offs);
        Serial.println(sca);
      }
      Serial.println(cmp);

      // simulate message
      String msg = String(tempOut,2) + "," + String(humOut,2) + "," + String(tempIn,2) + "," + String(batteryPercentage, 0); 
      for (int i= 0; i < 5; i++) {msg += ","; msg += String(weights[i],0);}
      Serial.print("LoRa Message:" );
      Serial.println(msg);

      Serial.print("\n\n");


      display.clearDisplay();
      display.setCursor(0,0);
      display.print("HIVE COM DEBUG");
      display.drawFastHLine(0, 10, 127,WHITE);
      display.setCursor(0,12);
      display.print(batMsg + "\n" + dhtMsg + "\n" + dsMsg + "\n" + sca2 + "\nWeight:" + String(weights[0],0) + "g");
      display.display();

      delay(300); 
    }
       
  }
}



void setup() {
  Serial.begin(115200);
  
  pinMode(ATTINY_POWER_PIN, OUTPUT);
  digitalWrite(ATTINY_POWER_PIN, true);
  delay(100);

  EEPROM.begin(1);
  measurement_interval_index = EEPROM.readUChar(0);
  measurement_interval_index %= 6;

  // initialize wire / I2C connection to ATtiny
  Wire.setClock(2000); //10kHz
  Wire.setTimeOut(200);
  Wire.begin();

  // initialize display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  display.setTextSize(1);

  showLogo();


  display.clearDisplay();
  display.setCursor(0,0);
  display.print("HIVE COM");
  display.drawFastHLine(0, 10, 127,WHITE);
  display.setCursor(0,12);
  display.print("Press button\nfor debug mode");
  display.display();

  

  loraSetup();

  delay(3000);

  // initialize other sensors
  dht22_sensor.begin();
  ds18b20_sensor.begin();
  pinMode(0, INPUT);
  pinMode(DEBUG_BUTTON_PIN, INPUT);

  esp_sleep_enable_timer_wakeup(measurement_intervals[measurement_interval_index]*1000*1000*60);


  if (digitalRead(DEBUG_BUTTON_PIN) == false) { // if debug button is pressed on startup
    debugAction();
    // function goes forever
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.print("HIVE COM");
  display.drawFastHLine(0, 10, 127,WHITE);
  display.setCursor(0,12);
  display.print("Starting upload mode.\nMeasurement\n interval:"); 
  display.print(measurement_intervals[measurement_interval_index]);
  display.print("min\nscreen turns off");
  display.display();

  delay(8000);

  display.clearDisplay();
  display.display();
}

void loop() {
  // collect sensor data
  digitalWrite(ATTINY_POWER_PIN, true); // turn on ATtiny spontaneously from ESP32-GPIO pin
  delay(8000); // wait until ATtiny is turned on and grooved in
  
  float tempOut = dht22_sensor.readTemperature(); // DHT22 temperature & humidity sensor (outside)
  float humOut = dht22_sensor.readHumidity();
  ds18b20_sensor.requestTemperatures(); // DS18B20 waterproof temperature sensor (inside)
  float tempIn = ds18b20_sensor.getTempCByIndex(0);  
  float batteryPercentage = getBatteryPercentage(); // Battery Percentage
  if (batteryPercentage == 0) { noBattery(); }
  int status = getWeight(); // get weight values

  digitalWrite(ATTINY_POWER_PIN, false);

  //Send values authenticated over lora
  String msg = String(tempOut,2) + "," + String(humOut,2) + "," + String(tempIn,2) + "," + String(batteryPercentage, 0); 
  for (int i= 0; i < 5; i++) {msg += ","; msg += String(weights[i],0);}
  String loraPacket = getPacket(msg);
  LoRa.beginPacket();
  LoRa.print(loraPacket);
  LoRa.endPacket();

  //delay(10000);
  esp_deep_sleep_start();
}
