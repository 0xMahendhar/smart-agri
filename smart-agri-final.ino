#define BLYNK_TEMPLATE_ID "TMPL3OFQJ3oEM"
#define BLYNK_TEMPLATE_NAME "SmartAgri"
#define BLYNK_AUTH_TOKEN "TOKEN-TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

// ----- Wi-Fi Credentials -----
char ssid[] = "WIFIName";          // Replace with your Wi-Fi SSID
char pass[] = "SecretWifiPassword";  // Replace with your Wi-Fi Password

// ----- DHT11 -----
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ----- Soil Moisture -----
#define SOIL_PIN 34
const int soilDry = 2480;   // Calibrate for dry soil
const int soilWet = 1010;   // Calibrate for wet soil

// ----- Gas Sensors -----
#define MQ2_PIN 32
#define MQ135_PIN 33

// ----- Blynk Virtual Pins -----
#define VPIN_TEMP V1
#define VPIN_HUM  V2
#define VPIN_SOIL V3
#define VPIN_COND V4
#define VPIN_MQ2_LPG V5
#define VPIN_MQ2_SMOKE V6
#define VPIN_MQ135_CO2 V7
#define VPIN_MQ135_ALCOHOL V8
#define VPIN_AQI V9

BlynkTimer timer;

// ---------- Wi-Fi + Blynk Reconnect ----------
void checkConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ Wi-Fi disconnected! Attempting reconnect...");
    WiFi.begin(ssid, pass);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) Serial.println("\n✅ Wi-Fi reconnected successfully.");
    else Serial.println("\n❌ Wi-Fi reconnect failed. Will retry...");
  }

  if (!Blynk.connected()) {
    Serial.println("⚠️ Blynk disconnected! Attempting reconnect...");
    if (Blynk.connect()) Serial.println("✅ Blynk reconnected successfully.");
    else Serial.println("❌ Blynk reconnect failed. Will retry...");
  }
}

// ---------- Read and Send Sensor Data ----------
void sendSensorData() {
  // --- DHT11 ---
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // --- Soil Moisture ---
  int soilRaw = analogRead(SOIL_PIN);
  float soilPercent = map(soilRaw, soilDry, soilWet, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  String soilCondition;
  if (soilPercent < 30)
    soilCondition = "Dry";
  else if (soilPercent < 70)
    soilCondition = "Moist";
  else
    soilCondition = "Wet";

  // --- MQ Sensors ---
  int mq2Raw = analogRead(MQ2_PIN);
  int mq135Raw = analogRead(MQ135_PIN);

  // Convert raw ADC to approximate ppm (linear placeholder)
  float mq2_LPG_ppm = mq2Raw * 1.0;       // calibrate as needed
  float mq2_Smoke_ppm = mq2Raw * 0.5;     // calibrate as needed
  float mq135_CO2_ppm = mq135Raw * 2.0;   // calibrate as needed
  float mq135_Alcohol_ppm = mq135Raw * 0.8; // calibrate as needed

  // AQI-like index (simplified)
  float aqiIndex = max(max(mq2_LPG_ppm, mq2_Smoke_ppm), max(mq135_CO2_ppm, mq135_Alcohol_ppm));
  aqiIndex = constrain(aqiIndex / 10.0, 0, 500);

  // ---- Print to Serial ----
  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print("\tHum: "); Serial.print(humidity);
  Serial.print("\tSoil Raw: "); Serial.print(soilRaw);
  Serial.print("\tSoil %: "); Serial.print(soilPercent);
  Serial.print("\tCond: "); Serial.print(soilCondition);
  Serial.print("\tMQ2 LPG: "); Serial.print(mq2_LPG_ppm);
  Serial.print("\tMQ2 Smoke: "); Serial.print(mq2_Smoke_ppm);
  Serial.print("\tMQ135 CO2: "); Serial.print(mq135_CO2_ppm);
  Serial.print("\tMQ135 Alcohol: "); Serial.print(mq135_Alcohol_ppm);
  Serial.print("\tAQI: "); Serial.println(aqiIndex);

  // ---- Send to Blynk ----
  if (Blynk.connected()) {
    Blynk.virtualWrite(VPIN_TEMP, temperature);
    Blynk.virtualWrite(VPIN_HUM, humidity);
    Blynk.virtualWrite(VPIN_SOIL, soilPercent);
    Blynk.virtualWrite(VPIN_COND, soilCondition);
    Blynk.virtualWrite(VPIN_MQ2_LPG, mq2_LPG_ppm);
    Blynk.virtualWrite(VPIN_MQ2_SMOKE, mq2_Smoke_ppm);
    Blynk.virtualWrite(VPIN_MQ135_CO2, mq135_CO2_ppm);
    Blynk.virtualWrite(VPIN_MQ135_ALCOHOL, mq135_Alcohol_ppm);
    Blynk.virtualWrite(VPIN_AQI, aqiIndex);
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(MQ2_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);

  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);

  Serial.println("SmartAgri: DHT11 + Soil + MQ2/MQ135 + Blynk + AQI Ready");

  timer.setInterval(2000L, sendSensorData);     // Send sensor data every 2 sec
  timer.setInterval(10000L, checkConnection);   // Check Wi-Fi/Blynk every 10 sec
}

// ---------- Loop ----------
void loop() {
  if (Blynk.connected()) Blynk.run();
  timer.run();
}
