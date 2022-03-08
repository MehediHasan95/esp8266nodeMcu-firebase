#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MLX90614.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#define REPORTING_PERIOD_MS 1000
#define FIREBASE_HOST "example.firebaseio.com"
#define FIREBASE_AUTH "token_or_secret"
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"

uint32_t tsLastReport = 0;

float heartRate, oxygen, temperature;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
PulseOximeter pox;

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  pox.begin();

  while (!Serial);
  if (!mlx.begin())
  {
    while (1);
  };
  Serial.println(mlx.readEmissivity());

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Firebase.set("sensor/HeartRate", 0);
  Firebase.set("sensor/Oxygen", 0);
  Firebase.set("sensor/Temperature", 0);
}

void loop()
{
  pox.update();

  heartRate = pox.getHeartRate();
  oxygen = pox.getSpO2();
  temperature = mlx.readObjectTempC();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS)
  {
    Serial.print("Heart-rate: ");
    Serial.print(heartRate);
    Serial.print("bpm | SpO2: ");
    Serial.print(oxygen);
    Serial.println("%");

    Serial.print("Temperature= ");
    Serial.print(temperature);
    Serial.println("*C");

    Serial.println();
    tsLastReport = millis();

    if (heartRate != 0)
    {
      Firebase.setFloat("sensor/HeartRate", heartRate);
      Firebase.setInt("sensor/Oxygen", oxygen);
      Firebase.setFloat("sensor/Temperature", temperature);
    }
  }
}
