
#include <SPI.h>
#include <MFRC522.h>
#include <OLED_I2C.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESP32Time.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
extern "C" {
#include "crypto/base64.h"
}
#include "tone.h"
#include "graphics.h"

#define SS_PIN  21
#define RST_PIN 22

int amount = 0;

WiFiMulti wifiMulti;
ESP32Time rtc;
MFRC522 mfrc522(SS_PIN, RST_PIN);
OLED  myOLED(5, 4); //(SDA, SCL)


char * WIFI_SSID = "__ssid__";
char * WIFI_PASS = "__password__";

//get your credentials from https://developer.safaricom.co.ke/user/me/apps
String consumer_key = "__consumer_key__";
String consumer_secret = "__consumer_secret__";
//https://developer.safaricom.co.ke/test_credentials
String business_code = "174379";
String pass_key = "__pass_key__";

String number = "2547XXXX__";

extern uint8_t SmallFont[], MediumNumbers[];

void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 1619827200) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
  myOLED.clrScr();
  myOLED.setFont(SmallFont);
  myOLED.print("Time set", LEFT, 0);
  myOLED.print(rtc.getTime("%H:%M:%S"), CENTER, 12);
  myOLED.print(rtc.getTime("%d-%m-%Y"), CENTER, 23);
  myOLED.drawBitmap(111, 0, wifi, 16, 16);
  myOLED.update();
}

void setup() {
  // put your setup code here, to run once
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(15, INPUT);

  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);

  if (!myOLED.begin(SSD1306_128X32))
    while (1);  // In case the library failed to allocate enough RAM for the display buffer...

  myOLED.setFont(SmallFont);

  Serial.print("Waiting for WiFi to connect...");
  while ((wifiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }
  myOLED.clrScr();
  myOLED.setFont(SmallFont);
  myOLED.print("WiFi Connected", LEFT, 12);
  myOLED.drawBitmap(111, 0, wifi, 16, 16);
  myOLED.update();

  setClock();

}

void loop() {
  // put your main code here, to run repeatedly:
  String uid = "";    // uid of the RFID card

  if (digitalRead(15) == HIGH) {
    amount = random(20, 100);   //generate random amount
    myOLED.setFont(MediumNumbers);
    myOLED.clrScr();
    myOLED.printNumI(amount, CENTER, 12);
    myOLED.setFont(SmallFont);
    myOLED.print("MPESA 1tap", LEFT, 0);
    myOLED.print("KSH", 80, 20);
    if ((wifiMulti.run() == WL_CONNECTED)) {
      myOLED.drawBitmap(111, 0, wifi, 16, 16);
    }
    myOLED.drawBitmap(0, 15, phone, 16, 16);
    myOLED.update();
  }

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {

    ToneOut(2000, 100);
    delay(10);
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      unsigned long id = (mfrc522.uid.uidByte[i]);
      uid = uid + id;
      Serial.print(id, HEX);
      Serial.print(" ");

    }
    Serial.println(" ");
    mfrc522.PICC_HaltA();
    Serial.println(uid);
    if (uid == "4811389014486128") {
      myOLED.setFont(MediumNumbers);
      myOLED.clrScr();
      myOLED.printNumI(amount, CENTER, 12);
      myOLED.setFont(SmallFont);
      myOLED.print("PAYING...", CENTER, 0);
      myOLED.print("KSH", 80, 20);
      if ((wifiMulti.run() == WL_CONNECTED)) {
        myOLED.drawBitmap(111, 0, wifi, 16, 16);
      }
      myOLED.drawBitmap(0, 15, timer, 16, 16);
      myOLED.update();
      if ((wifiMulti.run() == WL_CONNECTED)) {
        String token = getAccessToken();
        Serial.println(token);
        stkPush(token, number, String(amount));
      }

    }
    delay(1000);
  }

}

String encoder(String input) {
  char toEncode[input.length() + 1] ;
  input.toCharArray(toEncode, input.length() + 1);
  size_t outputLength;
  unsigned char * encoded = base64_encode((const unsigned char *)toEncode, strlen(toEncode), &outputLength);
  String encodedString = (const char*)encoded;
  free(encoded);
  encodedString.replace("\n", "");
  return encodedString;
}

String getAccessToken() {
  String token = "";
  String enc = encoder(consumer_key + ":" + consumer_secret);
  Serial.print("[");
  Serial.print(enc);
  Serial.println("]");
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");

  http.begin("https://sandbox.safaricom.co.ke/oauth/v1/generate?grant_type=client_credentials");
  http.addHeader("authorization", "Basic " + enc);
  http.addHeader("cache-control", "no-cache");
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      JSONVar myObject = JSON.parse(payload);
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return token;
      }
      // myObject.keys() can be used to get an array of all the keys in the object
      JSONVar keys = myObject.keys();

      for (int i = 0; i < keys.length(); i++) {
        JSONVar value = myObject[keys[i]];
        //        Serial.print(keys[i]);
        //        Serial.print(" = ");
        //        Serial.println(value);
        if (i == 0) {
          token = value;

        }
      }
      //Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return token;
}

void stkPush(String token, String number, String amount) {

  String timestamp = rtc.getTime("%Y%m%d%H%M%S");

  String password = encoder(business_code + pass_key + timestamp);

  String json = "{\"BusinessShortCode\":\"" + business_code + "\",\"Password\":\"" + password + "\",\"Timestamp\":\"" + timestamp + "\",\"TransactionType\":\"CustomerPayBillOnline\",\"Amount\":\"" + amount + "\",\"PartyA\":\"" + number + "\",\"PartyB\":\"" + business_code + "\",\"PhoneNumber\":\"" + number + "\",\"CallBackURL\":\"http://www.biego.tech/\",\"AccountReference\":\"ESP32\",\"TransactionDesc\":\"stktest\"}";

  HTTPClient http;

  Serial.print("[HTTP] begin...\n");

  http.begin("https://sandbox.safaricom.co.ke/mpesa/stkpush/v1/processrequest");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("authorization", "Bearer " + token);
  http.addHeader("cache-control", "no-cache");
  int httpCode = http.POST(json);

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {

      myOLED.clrScr();
      myOLED.setFont(SmallFont);
      myOLED.print("ENTER YOUR", CENTER, 0);
      myOLED.print("MPESA PIN", CENTER, 15);
      if ((wifiMulti.run() == WL_CONNECTED)) {
        myOLED.drawBitmap(111, 0, wifi, 16, 16);
      }
      myOLED.drawBitmap(0, 15, lock, 16, 16);
      myOLED.update();
      String payload = http.getString();
      JSONVar myObject = JSON.parse(payload);
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      // myObject.keys() can be used to get an array of all the keys in the object
      JSONVar keys = myObject.keys();

      for (int i = 0; i < keys.length(); i++) {
        JSONVar value = myObject[keys[i]];
        //        Serial.print(keys[i]);
        //        Serial.print(" = ");
        //        Serial.println(value);
      }
      //Serial.println(payload);
      
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
