#define BLYNK_TEMPLATE_ID   "TMPL3OFQJ3oEM"
#define BLYNK_AUTH_TOKEN    "tfum7sY1H0cd-Tbp3fl4KD0y77EnAHeT" // REPLACE
char ssid[] = "WIFI-Name";      // REPLACE
char pass[] = "WifiPassword";   // REPLACE

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Sensor & Actuator Definitions
#define SOIL_MOISTURE_PIN 34 // ADC pin for Soil Moisture Analog output
#define RELAY_PIN 27         // GPIO pin for controlling a relay (e.g., water pump)
#define SOIL_MOISTURE_VPIN V3 // Virtual Pin for Soil Moisture
#define RELAY_VIRTUAL_PIN V6 // Virtual Pin for Relay control

// Calibration values (Adjust if needed)
const int SENSOR_DRY = 3000;    // Raw ADC value when the sensor is dry (in air)
const int SENSOR_WET = 1500;    // Raw ADC value when the sensor is wet (in water)
const int ADC_MAX = 4095;       // ESP32 12-bit ADC max value

// Automation Thresholds
const int MOISTURE_THRESHOLD_ON = 40;  // Turn ON pump if moisture is below this %
const int MOISTURE_THRESHOLD_OFF = 60; // Turn OFF pump if moisture is above this %
const long FAST_SENSORS_INTERVAL = 5000L; // Read every 5s

BlynkTimer timer;

// Function to handle commands from Blynk App (Manual Control or Auto-Control)
BLYNK_WRITE(RELAY_VIRTUAL_PIN)
{
  int relayState = param.asInt();
  if (relayState == 1) {
    digitalWrite(RELAY_PIN, LOW); // Active-LOW relay ON
    Serial.println("Actuator (V6) ON via Blynk/Auto-Control");
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Active-LOW relay OFF
    Serial.println("Actuator (V6) OFF via Blynk/Auto-Control");
  }
}

void readSoilMoistureAndAutoControl()
{
  int soilValue = analogRead(SOIL_MOISTURE_PIN);

  if (soilValue >= ADC_MAX) {
    Serial.println("Error: Soil Moisture Sensor pin may be UNCONNECTED.");
    Blynk.virtualWrite(SOIL_MOISTURE_VPIN, "Error");
  } else {
    // Map raw ADC value to 0-100% moisture
    int moisturePercent;
    if (SENSOR_DRY > SENSOR_WET) {
      moisturePercent = map(soilValue, SENSOR_WET, SENSOR_DRY, 100, 0);
    } else {
      moisturePercent = map(soilValue, SENSOR_DRY, SENSOR_WET, 0, 100);
    }
    moisturePercent = constrain(moisturePercent, 0, 100);

    Serial.print("Soil Moisture Percent: "); Serial.print(moisturePercent); Serial.println("%");
    Blynk.virtualWrite(SOIL_MOISTURE_VPIN, moisturePercent); // Send to V3

    // Automated Irrigation Logic
    if (moisturePercent < MOISTURE_THRESHOLD_ON) {
        Blynk.virtualWrite(RELAY_VIRTUAL_PIN, 1); // Trigger ON
        Serial.println(">> AUTO-IRRIGATION ON: Moisture < 40%");
    } else if (moisturePercent >= MOISTURE_THRESHOLD_OFF) {
        Blynk.virtualWrite(RELAY_VIRTUAL_PIN, 0); // Trigger OFF
        Serial.println(">> AUTO-IRRIGATION OFF: Moisture >= 60%");
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Ensure pump is OFF initially (Active-LOW relay)

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(FAST_SENSORS_INTERVAL, readSoilMoistureAndAutoControl); // Read every 5s
}

void loop()
{
  Blynk.run();
  timer.run();
}
