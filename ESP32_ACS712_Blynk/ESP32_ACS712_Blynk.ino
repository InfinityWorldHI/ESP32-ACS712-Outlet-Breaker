#define BLYNK_PRINT Serial
//*****************Libraries*********************************
#include "ACS712.h"
//ESP32----------------------------
#include <WiFi.h>
#include <HTTPClient.h>
#include <SimpleTimer.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
//***********************************************************
#define TypeSen    ACS712_05B
#define pinAC      34
#define pinLED     26
#define pinRelay   25

#define VpinWatts  V0
#define VpinAmps   V1
#define VpinMaxW   V2
#define VpinMaxA   V3
#define VpinACV    V4

#define CHAT_ID "Telegram ChatID"
#define BOTtoken "123456789:Bot Token"

char auth[] = "Blynk Auth Token";
//***********************************************************
//ESP32 Temprerature
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
//***********************************************************
SimpleTimer timer;
ACS712 sensor(TypeSen, pinAC);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
//***********************************************************
/* Set these to your desired credentials. */
#define WiFiNumber 1           //Number of the WiFi networks
const char* ssid[] = {"SSID"};
const char* pass[] = {"Password"};
//***********************************************************
String getData, Link;

unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;
//***********************************************************
//User defined values
int ACvolts = 230;
double maxWatts = 50;
double maxAmps = 0;

bool WiFireconnect  = true;
bool Blynkreconnect = true;
bool sendTelegram = true;
//***********************************************************
BLYNK_WRITE(VpinMaxW) {
  maxWatts = param.asFloat();
}
//***********************************************************
BLYNK_WRITE(VpinMaxA) {
  maxAmps = param.asFloat();
}
//***********************************************************
BLYNK_WRITE(VpinACV) {
  ACvolts = param.asInt();
}
//***********************************************************
void SendNotif() {
  bot.sendMessage(CHAT_ID, "The power consumption is too HIGH, please check", "");
}
//***********************************************************
void SendData() {
  //---------------------------------------------
  float I = sensor.getCurrentAC();
  float P = ACvolts * I;
  //---------------------------------------------
  if (P > maxWatts && sendTelegram) {
    digitalWrite(pinRelay, HIGH);
    sendTelegram = false;
    SendNotif();
  }
  //---------------------------------------------
  Serial.println(String("I = ") + I + " A");
  Serial.println(String("P = ") + P + " Watts");
  Blynk.virtualWrite(VpinAmps, I);
  Blynk.virtualWrite(VpinWatts, P);
  //---------------------------------------------
  if ((millis() - previousMillis2) > 300000) {
    previousMillis2 = millis();
    sendTelegram = true;
  }
}
//***********************************************************
void setup() {
  Serial.begin(9600);
  //---------------------------------------------
  uint16_t acc = 0;
  for (int i = 0; i < 10; i++) {
    acc += analogRead(pinAC);
  }
  Serial.print("zero= ");
  Serial.println(acc / 10);
  Serial.print("calibrate= ");
  Serial.println(sensor.calibrate());
  //---------------------------------------------
  //  sensor.setZeroPoint(1813);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  connectToWiFi();
  //---------------------------------------------
  bot.sendMessage(CHAT_ID, "The Outlet is online.", "");
  //---------------------------------------------
  timer.setInterval(1000L, SendData);
  //---------------------------------------------
  digitalWrite(pinRelay, HIGH);
  //---------------------------------------------
  sensor.calibrate();
  //---------------------------------------------
  digitalWrite(pinRelay, HIGH);
  //---------------------------------------------
  Blynk.virtualWrite(VpinACV, ACvolts);
  delay(2000);
}
//***********************************************************
void loop() {
  CheckNet();
  timer.run();
  if (Blynk.connected()) {
    Blynk.run();
  }
  if (millis() - previousMillis3 >= 40000) {
    previousMillis3 = millis();
    WiFireconnect = true;
    Blynkreconnect = true;
  }
}
//***********************************************************
void CheckNet() {
  if (!Blynk.connected()) {
    if ( !WiFi.isConnected() ) {
      digitalWrite(pinLED, LOW);
      if (WiFireconnect) {
        WiFireconnect = false;
        connectToWiFi();
      }
    }
    else {
      digitalWrite(pinLED, HIGH);
      if (Blynkreconnect) {
        Blynkreconnect = false;
        Blynk.connect(6000);
      }
    }
  }
}
//********************connect to the WiFi*******************
void connectToWiFi() {
  for (int i = 0; i < WiFiNumber; i++) {
    WiFi.begin(ssid[i], pass[i]);
    Serial.print("\nTry Connecting to ");
    Serial.print(ssid[i]);

    unsigned long previousMillis0 = 0;
    bool state = true;
    uint32_t periodToConnect = 25000L;
    for (uint32_t StartToConnect = millis(); (millis() - StartToConnect) < periodToConnect;) {
      if ( WiFi.status() != WL_CONNECTED ) {
        if ((millis() - previousMillis0) > 300) {
          digitalWrite(pinLED, state);
          previousMillis0 = millis();
          state = !state;;
        }
        Serial.print(".");
        delay(200);
      } else {
        break;
      }
    }
    if (WiFi.isConnected()) {
      break;
    }
  }

  if ( !WiFi.isConnected() ) {
    Serial.println("\nFailed to connect to all the WiFi networks");
    digitalWrite(pinLED, LOW);
  }
  else {
    Serial.println("\nWiFi connected");
    for (int i = 0; i < 5; i++) {
      digitalWrite(pinLED, HIGH);
      delay(100);
      digitalWrite(pinLED, LOW);
      delay(100);
    }
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    Blynk.config(auth, IPAddress(139,59,206,133), 8080);
    if (!Blynk.connected()) {
      Blynk.connect(6000);
    }
  }
}
