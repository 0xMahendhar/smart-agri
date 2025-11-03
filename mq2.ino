#define BLYNK_TEMPLATE_ID   "TMPL3OFQJ3oEM"
#define BLYNK_AUTH_TOKEN    "tfum7sY1H0cd-Tbp3fl4KD0y77EnAHeT" // REPLACE
char ssid[] = "WIFI-Name";      // REPLACE
char pass[] = "WifiPassword";   // REPLACE

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Sensor Definitions
#define MQ2_PIN 35      // ADC pin for MQ-2 Gas Analog output
#define MQ2_VPIN V4     // Virtual Pin for MQ-2 Gas Sensor
const int ADC_MAX = 4095; // ESP32 12-bit ADC max value
const long FAST_SENSORS_INTERVAL = 5000L; // Read every 5s

BlynkTimer timer;

void readMQ2Sensor()
{
  int mq2Value = analogRead(MQ2_PIN);

  if (mq2Value >= ADC_MAX) {
    Serial.println("Error: MQ-2 Gas Sensor pin may be UNCONNECTED.");
    Blynk.virtualWrite(MQ2_VPIN, "Error");
  } else {
    Serial.print("MQ-2 (Methane) Raw: "); Serial.println(mq2Value);
    Blynk.virtualWrite(MQ2_VPIN, mq2Value); // Send to V4
  }
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(FAST_SENSORS_INTERVAL, readMQ2Sensor); // Read every 5s
}

void loop()
{
  Blynk.run();
  timer.run();
}
