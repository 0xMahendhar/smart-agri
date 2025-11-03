#define BLYNK_TEMPLATE_ID   "TMPL3OFQJ3oEM"
#define BLYNK_AUTH_TOKEN    "tfum7sY1H0cd-Tbp3fl4KD0y77EnAHeT" // REPLACE
char ssid[] = "WIFI-Name";      // REPLACE
char pass[] = "WifiPassword";   // REPLACE

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Sensor Definitions
#define DHTPIN 4        // The GPIO pin connected to the DHT11 data pin
#define DHTTYPE DHT11
#define TEMP_VPIN V1    // Virtual Pin for Temperature
#define HUMIDITY_VPIN V2 // Virtual Pin for Humidity
const long DHT_SENSOR_INTERVAL = 10000L; // Read DHT sensor every 10s

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

void readDHTSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Error: Failed to read from DHT sensor!");
    Blynk.virtualWrite(TEMP_VPIN, "Error");
    Blynk.virtualWrite(HUMIDITY_VPIN, "Error");
  } else {
    Serial.print("Humidity: "); Serial.print(h);
    Serial.print("%  Temperature: "); Serial.print(t); Serial.println("°C");
    Blynk.virtualWrite(TEMP_VPIN, t);     // Send temperature to V1
    Blynk.virtualWrite(HUMIDITY_VPIN, h); // Send humidity to V2
  }
}

void setup()
{
  Serial.begin(115200);
  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(DHT_SENSOR_INTERVAL, readDHTSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
}
