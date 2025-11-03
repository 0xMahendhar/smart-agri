#define BLYNK_TEMPLATE_ID   "TMPL3OFQJ3oEM"
#define BLYNK_AUTH_TOKEN    "tfum7sY1H0cd-Tbp3fl4KD0y77EnAHeT" // REPLACE
char ssid[] = "WIFI-Name";      // REPLACE
char pass[] = "WifiPassword";   // REPLACE

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Actuator Definitions
#define RELAY_PIN 27    // GPIO pin for controlling a relay (e.g., water pump)
#define RELAY_VIRTUAL_PIN V6 // Virtual Pin for Relay control

// Function to handle commands from Blynk App (Manual Control)
BLYNK_WRITE(RELAY_VIRTUAL_PIN)
{
  int relayState = param.asInt();

  // Most common relay modules are active-LOW (digitalWrite(LOW) turns them ON)
  if (relayState == 1) {
    digitalWrite(RELAY_PIN, LOW); // Active-LOW relay ON
    Serial.println("Actuator (V6) ON via Blynk/Manual Control");
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Active-LOW relay OFF
    Serial.println("Actuator (V6) OFF via Blynk/Manual Control");
  }
}

void setup()
{
  Serial.begin(115200);

  // Initialize Actuator
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Ensure pump is OFF initially (Active-LOW relay)

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop()
{
  Blynk.run();
}
