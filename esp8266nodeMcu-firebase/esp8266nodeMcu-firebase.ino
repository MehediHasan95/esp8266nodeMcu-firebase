#include <Arduino.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ESP8266WebServer.h"
#include <FirebaseArduino.h>
#define FIREBASE_HOST "esp8266-testing-8d30e-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "CnNAzPZwBdr9VBbrT7u1hNNRBXTbIcKu4nd8hBq0"
#define WIFI_SSID "Hasan"
#define WIFI_PASSWORD "76278704H"

PulseOximeter pox;

unsigned long previousMillis = 0;
const long interval = 1000;
volatile boolean heartBeatDetected = false;

void onBeatDetected()
{
  heartBeatDetected = true;
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  if (Firebase.failed())
  {
    Serial.print("setting / message failed: ");
    Serial.println(Firebase.error());
  }

  if (!pox.begin())
  {
    Serial.println("CONNECTION FAILED");
    for (;;);
  }
  else
  {
    Serial.println("CONNECTION SUCCESSFULLY");
  }

 pox.setOnBeatDetectedCallback(onBeatDetected);
}

void oxiMeter()
{
  float bpm = pox.getHeartRate();
  float SpO2 = pox.getSpO2();

  if ( bpm != 0)
  {
    if (SpO2 > 0)
    {
      Firebase.setFloat("Heart-rate/BPM", bpm);
      Firebase.setFloat("Heart-rate/SpO2", SpO2);
      Serial.print("Heart-rate: ");
      Serial.print(bpm);
      Serial.print("bpm | SpO2: ");
      Serial.print(SpO2);
      Serial.println("%");

    }
  }
}

void loop()
{
  pox.update();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    pox.shutdown();
    oxiMeter();
    pox.resume();
    previousMillis = currentMillis;
  }
}
