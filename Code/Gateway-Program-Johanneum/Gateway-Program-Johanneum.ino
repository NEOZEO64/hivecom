/* LORA HIVE Gateway Software

1. Initialize sensors, lora interface, wait for wifi network
2. Loop
  a. If Lora package received
    i. Parse package
    ii. Upload values to influx bucket
    iii. upload a lot of maintenance infos
  b. Execute functions if button pressed

- function left-button: show data values of last received package
- function middle-button: show log
- function right-button: show rick-roll video

So this still is a lot of spagetti code to kind of keep the UI and the LoRa Service running, but it works for now

By the way, this script still talks a lot over Serial for debugging purposes
*/

#include "video.h"
#include "hivecomlogo.h"
#include "loraTools.h"
#include "oledTools.h"

#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <time.h>

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

extern "C" {uint8_t temprature_sens_read();}


const char* ssid = "ENTER-YOUR-SSID"; // your ssid
const char* EAP_ID = ""; 
const char* EAP_ANONYMOUS_IDENTITY = "anonymous@example.com"; //, or you can use also nickname@example.com
const char* EAP_USERNAME = "YOUR-EMAIL-ADRESS";
const char* EAP_PASSWORD = "YOUR-PASSWORD";

#define INFLUXDB_URL "https://influx.neozeo.de"
#define INFLUXDB_TOKEN "yesyesheresthetokenbutImNotGonnaShowYou"
#define INFLUXDB_ORG "YOUR_ORGANISATION_NAME"
#define INFLUXDB_BUCKET "YOUR_BUCKET_NAME"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3" // time zone info for Germany, why is it so complicated?
//time zone got from https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

String hostname = "YOUR_HOSTNAME";

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare data point
Point sensor("bee_hive");

float tempOut, humOut, tempIn, weight, weight1, weight2, weight3, weight4;
int bat, lastPacketSuccessful, packetSize, auth, wifirssi;
String lastSuccessfulTime = "no packet received\n";

int bl = 35; // button left pin
int bm = 14; // button middle pin
int br = 13; // button right pin

void showInverseLoad(float time) { // time in ms
  float step = 2.8*10*128/time;
  for (float i = 128; i > 0; i -= step) {
    display.drawFastHLine(0, 63, (int)i,SSD1306_WHITE); 
    display.drawFastHLine((int)i+1, 63, 128-(int)i,SSD1306_BLACK); 
    display.display(); // Update screen with each newly-drawn rectangle
    delay(10);
  }

  display.clearDisplay();
  display.setCursor(0,32);
  display.print("continue service");
  display.display();

  delay(2000);

  display.clearDisplay();
  display.display();
}

void showSensorStats() {
  // show last sensor stats & time of last successful upload
  String msg = "Last sensor stats\n";
  msg += lastSuccessfulTime;
  msg += String("OUTSIDE: ") + String(tempOut,2) + "C," + String(humOut,2) + "%\n";
  msg += String("INSIDE: ") + String(tempIn,2) + "C\n";
  msg += String("BATTERY STATUS: ") + String(bat) + "%\n";
  msg += String("WEIGHT SUM: ") + String(weight/1000, 3) + "kg\n"; 
  msg += String("WC1,2:") + String(weight1,0) + "g," + String(weight2,0) + "g,\n";
  msg += String("WC3,4:") + String(weight3,0) + "g," + String(weight4,0) + "g\n";
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(msg);
  display.display();

  Serial.println(msg);

  showInverseLoad(6000);
}



void showVideo() {
  // Show Rick Roll video
  for (long i = 0; i<168; i++) {
    display.clearDisplay();
    display.drawBitmap(0, 0, video+16*64*i, 128, 64, 1);
    display.display();
    delay(40);
  }
  showInverseLoad(1000);
}

float getCPUTemp() { return (temprature_sens_read() - 32) / 1.8; }

int parsePayloadValues(String payload) {
  Serial.println("Values got:");
  int nextCommaIndex;

  nextCommaIndex = payload.indexOf(",");
  if (nextCommaIndex <= 0) return 0;
  tempOut = payload.substring(0, nextCommaIndex).toFloat();
  payload = payload.substring(payload.indexOf(",")+1);

  nextCommaIndex = payload.indexOf(",");
  if (nextCommaIndex <= 0) return 0;
  humOut = payload.substring(0, nextCommaIndex).toFloat();
  payload = payload.substring(payload.indexOf(",")+1);
  
  nextCommaIndex = payload.indexOf(",");
  if (nextCommaIndex <= 0) return 0;
  tempIn = payload.substring(0, nextCommaIndex).toFloat();
  payload = payload.substring(nextCommaIndex+1);
  
  nextCommaIndex = payload.indexOf(",");
  if (nextCommaIndex <= 0) return 0;
  bat = payload.substring(0, nextCommaIndex).toFloat();
  payload = payload.substring(nextCommaIndex+1);
  
  nextCommaIndex = payload.indexOf(",");
  if (nextCommaIndex <= 0) return 0;
  weight = payload.substring(0, nextCommaIndex).toFloat();
  payload = payload.substring(nextCommaIndex+1);
  
  nextCommaIndex = payload.indexOf(",");
  if (nextCommaIndex <= 0) return 0;
  weight1 = payload.substring(0, nextCommaIndex).toFloat();
  payload = payload.substring(nextCommaIndex+1);
  
  nextCommaIndex = payload.indexOf(",");
  if (nextCommaIndex <= 0) return 0;
  weight2 = payload.substring(0, nextCommaIndex).toFloat();
  payload = payload.substring(nextCommaIndex+1);
  
  nextCommaIndex = payload.indexOf(",");
  if (nextCommaIndex <= 0) return 0;
  weight3 = payload.substring(0, nextCommaIndex).toFloat();
  payload = payload.substring(nextCommaIndex+1);
  
  weight4 = payload.toFloat();

  return 1;
}

String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}

void showLogo() {
  display.clearDisplay();
  display.drawBitmap(0, 0, hivecomlogo, 128, 64, 1); // offset the video pointer 16*64 bytes to get to the right image

  int x = 76;
  int y = 48;

  for (int t = 0; t < 20; t += 1) {
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


void setup() {
  oledSetup("HIVECOM GATEWAY");
  
  showLogo();

  loraSetup();
  Serial.begin(115200);

  pinMode(bl, INPUT_PULLUP);
  pinMode(bm, INPUT_PULLUP);
  pinMode(br, INPUT_PULLUP);

  // Setup WIFI
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  screenPrintln(INFO, "Connect " + (String) ssid);

  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_ID, EAP_USERNAME, EAP_PASSWORD);
  esp_wifi_sta_wpa2_ent_enable();
  WiFi.begin(ssid);

  int counter = 0;
  bool connected = false;
  while (!connected) {
    delay(500);
    switch (WiFi.status()){
    case 0:
      Serial.println("idle");
      screenPrintln(INFO, "idle");
      break;
    case 1:
      screenPrintln(INFO, "could not find wifi");
      Serial.println("could not find wifi with specified ssid");
      break;
    case 2:
      screenPrintln(INFO, "scanning completed");
      Serial.println("scanning completed");
      break;
    case 3:
      connected = true;
      screenPrintln(INFO, "connected");
      Serial.println("connected");
      break;
    case 4:
      screenPrintln(INFO, "connection failed");
      Serial.println("connection failed");
      break;
    case 5:
      screenPrintln(INFO, "connection lost");
      Serial.println("connection lost");
      break;
    case 6:
      screenPrintln(INFO, "disconnected");
      Serial.println("disconnected");
      break;
    }

    delay(800);

    counter ++;
    if (counter > 100) { //after 30 seconds timeout - reset board
      WiFi.disconnect();
      delay(100);
      ESP.restart();
    }
  }

  

  screenPrintln(INFO, "Connected");  
  screenPrintln(INFO, "Sync time now");  

  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");


  // Check server connection
  if (client.validateConnection()) {
    screenPrintln(INFO, "Connected to DB:" + (String)client.getServerUrl());
  } else {
    String error = client.getLastErrorMessage();
    screenPrintln(INFO, "DB err:");
    screenPrintln(INFO, error);
    Serial.print("InfluxDB connection failed: ");
    Serial.println(error);
  }

  // Add tags to the data point
  sensor.addTag("device", "LORA-HIVE");

  //Say hello 
  String hellomsg = "Service started";
  String helloPackage = getPacket(hellomsg);
  screenPrintln(SEND, hellomsg);

  showInverseLoad(3000);
}

void loop() {
  if (!(analogRead(bl)>500)) { Serial.println("left button"); showSensorStats(); } // if left button pressed
  if (!digitalRead(bm)) { Serial.println("middle button"); showLog(); showInverseLoad(6000);} // if middle button pressed
  if (!digitalRead(br)) { Serial.println("right button"); showVideo(); } // if right button pressed

  // #############################################################################
  // If LoRa packet received
  packetSize = LoRa.parsePacket();
  if (packetSize) {
    display.clearDisplay();
    display.setCursor(0,32);
    display.print("processing packet");
    display.display();


    Serial.println("Package received");
    String packet = "";
    while (LoRa.available()) {packet = LoRa.readString();}
    int rssi = LoRa.packetRssi();
    Serial.print("With RSSI ");
    Serial.println(rssi);
    String payload = "";
    String hash = "";
    auth = checkAuth(packet, &payload, &hash);
    IPAddress ip = WiFi.localIP();


    sensor.clearFields();
    sensor.addField("packetSize", packetSize);
    sensor.addField("gatewayCPUTemp", getCPUTemp());
    sensor.addField("auth", auth);
    sensor.addField("rssi", rssi);
    sensor.addField("lastPackSuccessfulInfluxUpl", lastPacketSuccessful);
    sensor.addField("wifiRssi", WiFi.RSSI());
    sensor.addField("localIp", IpAddress2String(ip));
    
    if (auth) {
      logln(RECEIVE, "Received data");
      int parseSuccessful = parsePayloadValues(payload);
      sensor.addField("parseSuccessful", parseSuccessful);
      sensor.addField("payload", payload);

      if (parseSuccessful) { //if payload parsing successfull
        sensor.addField("tempout", tempOut);
        sensor.addField("humout", humOut);
        sensor.addField("tempin", tempIn);
        sensor.addField("bat", bat);
        sensor.addField("weight", weight);
        sensor.addField("weight1", weight1);
        sensor.addField("weight2", weight2);
        sensor.addField("weight3", weight3);
        sensor.addField("weight4", weight4);

  
        // Write point
        lastPacketSuccessful = client.writePoint(sensor);
        if (lastPacketSuccessful) {
          logln(INFO, "Uploaded to DB yay");

          Serial.print("Writing to InfluxDB: ");
          Serial.println(sensor.toLineProtocol());
          Serial.println("\n");

          // save timestamp of successful packet
          time_t tnow = time(nullptr);
          lastSuccessfulTime = ctime(&tnow);

        } else {
          String error = client.getLastErrorMessage();
          logln(INFO, "!DB err:" + error);
          Serial.println("InfluxDB write failed: " + error);
        }
      } else {
        logln(INFO, "!Payload parsing error");
        sensor.addField("parseSuccessful", 0);
      }
    } else {
      // no authenticated message detected!
      sensor.addField("payload", packet);
      lastPacketSuccessful = client.writePoint(sensor);
      if (lastPacketSuccessful) {
          logln(INFO, "Uploaded to DB yay");
          Serial.println("Writing to InfluxDB: " + sensor.toLineProtocol());
      } else {
        String error = client.getLastErrorMessage();
        logln(INFO, "!DB err:" + error);
        Serial.println("InfluxDB write failed: " + error);
      }

      logln(ECHO, (String)"!" + packet);
    }

    display.clearDisplay();
    display.display();

  }

  // Check WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    logln(INFO, "!No Wifi");
  }
}