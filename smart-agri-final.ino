/************************************************************
  🌾 SmartAgri Modular ESP32
  Sensors: DHT11, Soil Moisture, MQ2, MQ135
  Features: Modular sensor functions, Blynk integration, 
            Dynamic interval slider, Safe Wi-Fi & Blynk reconnect
************************************************************/

// ---------- Blynk Template & Auth ----------
#define BLYNK_TEMPLATE_ID "TMPL3OFQJ3oEM"   // Replace with your Template ID
#define BLYNK_TEMPLATE_NAME "SmartAgri"     // Replace with your Template Name
#define BLYNK_AUTH_TOKEN "TOKEN"  // Device Auth Token

// ---------- Libraries ----------
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

// ---------- Wi-Fi Credentials ----------
char ssid[] = "WIFI";          
char pass[] = "WIFI Password";  

// ---------- DHT11 ----------
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------- Soil Moisture ----------
#define SOIL_PIN 34
const int soilDry = 2480;
const int soilWet = 1010;

// ---------- MQ Sensors ----------
#define MQ2_PIN 32
#define MQ135_PIN 33

// ---------- Relay (Optional) ----------
#define RELAY_PIN 26

// ---------- Blynk Virtual Pins ----------
#define VPIN_TEMP V1
#define VPIN_HUM  V2
#define VPIN_SOIL V3
#define VPIN_COND V4
#define VPIN_MQ2_LPG V5
#define VPIN_MQ2_SMOKE V6
#define VPIN_MQ135_CO2 V7
#define VPIN_MQ135_ALCOHOL V8
#define VPIN_AQI V9
#define VPIN_INTERVAL V10   // Slider for dynamic interval

BlynkTimer timer;
int intervalSeconds = 900; // default 15 minutes
int sensorTimerID;
bool wifiConnecting = false;

// ---------- Sensor Data Structures ----------
struct DHTData { float temperature; float humidity; };
struct SoilData { float percent; String condition; };
struct MQData { float mq2_LPG, mq2_Smoke, mq135_CO2, mq135_Alcohol, aqi; };

// ---------- Sensor Read Functions ----------
DHTData readDHT() {
  DHTData data;
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  return data;
}

SoilData readSoil() {
  SoilData data;
  int soilRaw = analogRead(SOIL_PIN);
  data.percent = map(soilRaw, soilDry, soilWet, 0, 100);
  data.percent = constrain(data.percent, 0, 100);
  data.condition = (data.percent < 30) ? "Dry" : (data.percent < 70) ? "Moist" : "Wet";
  return data;
}

MQData readMQSensors() {
  MQData data;
  int mq2Raw = analogRead(MQ2_PIN);
  int mq135Raw = analogRead(MQ135_PIN);
  data.mq2_LPG = mq2Raw * 1.0;
  data.mq2_Smoke = mq2Raw * 0.5;
  data.mq135_CO2 = mq135Raw * 2.0;
  data.mq135_Alcohol = mq135Raw * 0.8;
  data.aqi = max(max(data.mq2_LPG, data.mq2_Smoke), max(data.mq135_CO2, data.mq135_Alcohol));
  data.aqi = constrain(data.aqi / 10.0, 0, 500);
  return data;
}

// ---------- Output Functions ----------
void sendToSerial(DHTData dhtData, SoilData soilData, MQData mqData) {
  Serial.printf("🌡 Temp: %.1f°C  Hum: %.1f%%  Soil: %.1f%%(%s)\n",
                dhtData.temperature, dhtData.humidity,
                soilData.percent, soilData.condition.c_str());
  Serial.printf("🔥 MQ2 LPG: %.1f  🚭 MQ2 Smoke: %.1f\n", mqData.mq2_LPG, mqData.mq2_Smoke);
  Serial.printf("💨 MQ135 CO2: %.1f  🥃 MQ135 Alcohol: %.1f  AQI: %.1f\n",
                mqData.mq135_CO2, mqData.mq135_Alcohol, mqData.aqi);
  Serial.printf("==========x===========x======== \n");
}

void sendToBlynk(DHTData dhtData, SoilData soilData, MQData mqData) {
  if (!Blynk.connected()) return;
  Blynk.virtualWrite(VPIN_TEMP, dhtData.temperature);
  Blynk.virtualWrite(VPIN_HUM, dhtData.humidity);
  Blynk.virtualWrite(VPIN_SOIL, soilData.percent);
  Blynk.virtualWrite(VPIN_COND, soilData.condition);
  Blynk.virtualWrite(VPIN_MQ2_LPG, mqData.mq2_LPG);
  Blynk.virtualWrite(VPIN_MQ2_SMOKE, mqData.mq2_Smoke);
  Blynk.virtualWrite(VPIN_MQ135_CO2, mqData.mq135_CO2);
  Blynk.virtualWrite(VPIN_MQ135_ALCOHOL, mqData.mq135_Alcohol);
  Blynk.virtualWrite(VPIN_AQI, mqData.aqi);
}

// ---------- Sensor Task ----------
void readAndSendSensors() {
  DHTData dhtData = readDHT();
  SoilData soilData = readSoil();
  MQData mqData = readMQSensors();
  sendToSerial(dhtData, soilData, mqData);
  sendToBlynk(dhtData, soilData, mqData);
}

// ---------- Wi-Fi & Blynk Reconnect ----------
void checkConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    if (!wifiConnecting) {
      Serial.println("⚠️ Wi-Fi disconnected! Reconnecting...");
      WiFi.begin(ssid, pass);
      wifiConnecting = true;
    }
  } else {
    if (wifiConnecting) {
      Serial.println("✅ Wi-Fi reconnected");
      wifiConnecting = false;
    }
    if (!Blynk.connected()) {
      Serial.println("⚠️ Blynk disconnected! Reconnecting...");
      if (!Blynk.connect(10000)) { // try 10 seconds
        Serial.println("❌ Blynk reconnect failed, will retry...");
      } else {
        Serial.println("✅ Blynk reconnected successfully");
      }
    }
  }
}

// ---------- Slider for Dynamic Interval ----------
BLYNK_WRITE(VPIN_INTERVAL) {
  int newInterval = param.asInt();
  newInterval = constrain(newInterval, 5, 900); // 5 sec → 15 min
  if (newInterval != intervalSeconds) {
    intervalSeconds = newInterval;
    timer.deleteTimer(sensorTimerID);
    sensorTimerID = timer.setInterval(intervalSeconds * 1000L, readAndSendSensors);
    Serial.printf("⏱️ Interval changed: %d sec\n", intervalSeconds);
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(MQ2_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin(ssid, pass);

  Blynk.config(BLYNK_AUTH_TOKEN);
  Serial.println("🌾 SmartAgri Ready");

  // Check Wi-Fi & Blynk every 30 sec
  timer.setInterval(30000L, checkConnection);

  // Sensor read + Blynk/Serial output
  sensorTimerID = timer.setInterval(intervalSeconds * 1000L, readAndSendSensors);

  // Sync slider with default interval
  Blynk.virtualWrite(VPIN_INTERVAL, intervalSeconds);
}

// ---------- Loop ----------
void loop() {
  if (Blynk.connected()) Blynk.run();
  timer.run();
}
