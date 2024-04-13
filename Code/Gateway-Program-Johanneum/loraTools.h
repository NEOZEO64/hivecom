#include <SPI.h> //Libraries for LoRa
#include <LoRa.h>
#include "mbedtls/md.h" // for using hashing algorithms
mbedtls_md_context_t ctx;

//for authenticating lora messages with hashes
String key = "nonoImNotGonnaGiveYouMyPassword!";

String getHash(String payload) { //turns first 32 characters into hash
  char payloadBuffer[payload.length() + 1];
  payload.toCharArray(payloadBuffer, payload.length() + 1);

  byte shaResultBuffer[32];
  String result = "";
  
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0); //0 for not using HMAC (can be used if set != 0)
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char*) payloadBuffer, payload.length());
  mbedtls_md_finish(&ctx, shaResultBuffer);
  mbedtls_md_free(&ctx);

  for (int i = 0; i < sizeof(shaResultBuffer); i++) {
    char str[3];
    sprintf(str, "%02x", (int)shaResultBuffer[i]);
    result += String(str);
  }
  return result;
}

//put the payload into packet with hash
String getPacket(String payload) {
  String package =  payload + String("#") + getHash(payload+key);
  Serial.print("Built package:");
  Serial.println(package);
  return package;
}

bool checkAuth(String packet, String* payloadOut = NULL, String* hashOut = NULL) { //returns true if hash fits to seed
  int hashtagCharPos = packet.indexOf("#");
  if (hashtagCharPos <= 0) {return false;}
  
  String payload = packet.substring(0,hashtagCharPos);
  String hash = packet.substring(hashtagCharPos+1,packet.length());

  String calculatedHash = getHash(payload+key);

  Serial.print("Checking authentication of:");
  Serial.println(packet);
  Serial.print("Payload is:");
  Serial.println(payload);
  Serial.print("Package hash is:");
  Serial.println(hash);
  Serial.print("Calculated hash:");
  Serial.println(calculatedHash);

  if (payloadOut != NULL && hashOut != NULL) {(*payloadOut) = payload; (*hashOut) = hash;}

  bool result;
  if (hashtagCharPos >= 1000) {
    Serial.println("Message too long, no auth");

    result = false;
  } else if (calculatedHash == hash) { // received hash compared with own calculated one
    Serial.println("Hashes equal, message is authenticated");
    result = true;
  } else { // wrong hash! error from packet loss or did some one else send the package?
    Serial.println("Hashes not equal, message is not authenticated");
    result = false;
  } 

  return result;
}

void loraSetup() {
  //setup LoRa
  SPI.begin(5, 19, 27, 18); //SCK, MISO, MOSI, SS
  LoRa.setPins(18, 23, 26); //SS, RST, DIO0
  LoRa.begin(866E6); //866E6 for EU frequency band
  /*
  Spreading factor:
  7 for low energy consumption, low range (2km), high data rate
  10 for high energy consumption, high range (8km), low data rate
  ...until 12
  */
  LoRa.setSpreadingFactor(12);
}