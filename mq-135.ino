#define BLYNK_TEMPLATE_ID   "TMPL3OFQJ3oEM"
#define BLYNK_AUTH_TOKEN    "tfum7sY1H0cd-Tbp3fl4KD0y77EnAHeT" // REPLACE
char ssid[] = "WIFI-Name";      // REPLACE
char pass[] = "WifiPassword";   // REPLACE

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Sensor Definitions
#define MQ135_PIN 32    // ADC pin for MQ-135 Gas Analog output
#define MQ135_VPIN V5   // Virtual Pin for MQ-135 Gas Sensor
const int ADC_MAX = 4095; // ESP32 12-bit ADC max value
const long FAST_SENSORS_INTERVAL = 5000L; // Read every 5s

BlynkTimer timer;

void readMQ135Sensor()
{
  int mq135Value = analogRead(MQ135_PIN);

  if (mq135Value >= ADC_MAX) {
    Serial.println("Error: MQ-135 Gas Sensor pin may be UNCONNECTED.");
    Blynk.virtualWrite(MQ135_VPIN, "Error");
  } else {
    Serial.print("MQ-135 (Air Quality) Raw: "); Serial.println(mq135Value);
    Blynk.virtualWrite(MQ135_VPIN, mq135Value); // Send to V5
  }
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(FAST_SENSORS_INTERVAL, readMQ135Sensor); // Read every 5s
}

void loop()
{
  Blynk.run();
  timer.run();
}
