#include <MAX30100_BeatDetector.h>
#include <MAX30100_Filters.h>
#include <MAX30100_PulseOximeter.h>
#include <MAX30100_Registers.h>
#include <MAX30100_SpO2Calculator.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ESP8266WebServer.h"
#include <FirebaseArduino.h>
#define FIREBASE_HOST "esp8266-testing-8d30e-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "CnNAzPZwBdr9VBbrT7u1hNNRBXTbIcKu4nd8hBq0"
#define WIFI_SSID "Hasan"
#define WIFI_PASSWORD "76278704H"
#define REPORTING_PERIOD_MS 500

PulseOximeter pox;

const int numReadings = 10;
float filterweight = 0.5;
uint32_t tsLastReport = 0;
uint32_t last_beat = 0;
int readIndex = 0;
float average_beat = 0;
float average_SpO2 = 0;
bool calculation_complete = false;
bool calculating = false;
bool initialized = false;
byte beat = 0;

void onBeatDetected()
{
  viewBeat();
  last_beat = millis();
}

void viewBeat()
{
  if (beat == 0)
  {
    Serial.print("💔");
    beat = 1;
  }
  else
  {
    Serial.print("❤");
    beat = 0;
  }
}

void initial_display()
{
  if (not initialized)
  {
    viewBeat();
    Serial.println("PLEASE PLACE YOUR FINGER ON THE SENSOR");
    initialized = true;
  }
}

void display_calculating(int j)
{
  viewBeat();
  Serial.println("MEASURING");
  for (int i = 0; i <= j; i++)
  {
    Serial.print("");
  }
}

void display_values()
{
  if (average_beat > 30 and average_SpO2 > 50)
  {
    Firebase.pushFloat("Heart-rate/BPM", average_beat);
    Firebase.pushInt("Heart-rate/SpO2", average_SpO2);
  }

  Serial.print(average_beat);
  Serial.print("bpm | SpO2: ");
  Serial.print(average_SpO2);
  Serial.print("%");
}

void calculate_average(float beat, int SpO2)
{
  if (readIndex == numReadings)
  {
    calculation_complete = true;
    calculating = false;
    initialized = false;
    readIndex = 0;
    display_values();
  }

  if (not calculation_complete and beat > 30 and beat < 220 and SpO2 > 50)
  {
    average_beat = filterweight * (beat) + (1 - filterweight) * average_beat;
    average_SpO2 = filterweight * (SpO2) + (1 - filterweight) * average_SpO2;
    readIndex++;
    display_calculating(readIndex);
  }
}

void firebaseInitialize()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("CONNECTING");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("📡");
    delay(500);
  }

  Serial.println();
  Serial.print("CONNECTED WITH IP: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if (Firebase.failed())
  {
    Serial.print("SETTING / MESSAGE FAILED: ");
    Serial.println(Firebase.error());
  }
}

void setup()
{
  Serial.begin(115200);

  firebaseInitialize();

  pox.begin();
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  pox.update();
  if ((millis() - tsLastReport > REPORTING_PERIOD_MS) and (not calculation_complete))
  {
    calculate_average(pox.getHeartRate(), pox.getSpO2());
    tsLastReport = millis();
  }

  if ((millis() - last_beat > 10000))
  {
    calculation_complete = false;
    average_beat = 0;
    average_SpO2 = 0;
    initial_display();
  }
}
