
#include <SPI.h>
#include <MFRC522.h>
#include <OLED_I2C.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <MpesaSTK.h>
#include "tone.h"
#include "graphics.h"

#define SS_PIN  21
#define RST_PIN 22

int amount = 0;

char * WIFI_SSID = "__ssid__";
char * WIFI_PASS = "__password__";

//get your credentials from https://developer.safaricom.co.ke/user/me/apps
String consumer_key = "__consumer_key__";
String consumer_secret = "__consumer_secret__";
//https://developer.safaricom.co.ke/test_credentials
int business_code = 174379;
String pass_key = "__pass_key__";

WiFiMulti wifiMulti;
MFRC522 mfrc522(SS_PIN, RST_PIN);
OLED  myOLED(5, 4); //(SDA, SCL)
MpesaSTK mpesa(consumer_key, consumer_secret, pass_key, SANDBOX);

String number = "2547XXXX__";

extern uint8_t SmallFont[], MediumNumbers[];


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

  mpesa.begin(business_code, PAYBILL, "http://mycallbackurl.com/checkout.php");

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
        String result = mpesa.pay(number, amount, "Arduino", "Test");
        Serial.println(result);
      }

    }
    delay(1000);
  }

}
