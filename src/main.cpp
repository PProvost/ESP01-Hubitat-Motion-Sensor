//#include <ESP8266WiFi.h>
#include <SmartThingsESP8266WiFi.h>
using HubitatClient = st::SmartThingsESP8266WiFi;

#include <ArduinoJson.h>


// Enable VCC measurement (must be at the top of the file in global scope)
ADC_MODE(ADC_VCC);

////////////////////////////////////////////////////////////////////////////////
// Configuration
////////////////////////////////////////////////////////////////////////////////

// NOTE: This file is NOT included in the git repo. You must create it and put
// it in the correct location, or replace this line with the equivalent defines.
// A sample file is provided in the file src/ap_setting_h.example.
#include "../../ap_setting.h"

// Static IP address enables faster startup (== less power consumption)
IPAddress ip_static(192, 168, 135, 98); // Must match what you set in the Hubitat Device config
IPAddress ip_gateway(192, 168, 135, 1);
IPAddress ip_subnet(255, 255, 255, 0);
IPAddress ip_dns(192, 168, 135, 1);

// This must be set to your Hubitat's static IP address
IPAddress ip_hub(192, 168, 135, 2);

// Must match what you set in the Hubitat Device config
#define DEVICE_PORT 8090

// NOTE: Change this to 39500 if you're using SmartThings instead of Hubitat
#define HUBITAT_PORT 39501

// defines the hold pin (will hold CH_PD high until we are ready to power down
#define HOLD_PIN 0

// defines the PIR read pin (reads the state of the PIR output)
#define PIR_DATA_PIN 3

// You might need to lower this depending on your USB-TTL-UART adapter and/or
// line noise
#define SERIAL_SPEED 9600

// Controls the minimum amount of time (milliseconds) the motion sensor shows  "active" state
#define MIN_ACTIVE_TIME 5000

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////

// Forward function declarations
static void messageCallout(String message);
static float getVccPercent();

// Flag for special processing for when we first wake up (first loop)
bool firstLoop = true;

// Local WiFi constants are defined in ap_setting.h, the others are defined
// above
HubitatClient hubClient(WIFI_SSID, WIFI_PASSWORD, ip_static,
                                      ip_gateway, ip_subnet, ip_dns,
                                      DEVICE_PORT, ip_hub, HUBITAT_PORT,
                                      messageCallout);

////////////////////////////////////////////////////////////////////////////////
// Arduino Framework Entry Points
////////////////////////////////////////////////////////////////////////////////

void setup()
{
  // Set HOLD_PIN to high (this holds CH_PD high)
  pinMode(HOLD_PIN, OUTPUT);
  digitalWrite(HOLD_PIN, HIGH);

  // If we're using RX or TX for GPIO, we need to set it to FUNCTION_3
  // before setting it up normally
  if (PIR_DATA_PIN == 1 || PIR_DATA_PIN == 3)
    pinMode(PIR_DATA_PIN, FUNCTION_3);
  pinMode(PIR_DATA_PIN, INPUT);

  // Start Serial connection. TX only if you're using RX for GPIO
  Serial.begin(SERIAL_SPEED, SERIAL_8N1, SERIAL_TX_ONLY);

  // Setup WiFi and hub connection
  hubClient.init();
  yield();
}

void loop()
{
  StaticJsonBuffer<256> jsonBuffer;
  char stringBuffer[256];
  JsonObject &root = jsonBuffer.createObject();

  float vcc = getVccPercent();

  if (firstLoop)
  {
    root["motion"] = "active";
    root["battery"] = vcc;
    root.printTo(stringBuffer);

    hubClient.send(stringBuffer);
    hubClient.run();
    yield();

    Serial.printf("Sent: %s\r\n", stringBuffer);

    unsigned long startMillis = millis();
    // Simple debounce of PIR triggering... wait a bit until the pin reads low
    while (digitalRead(PIR_DATA_PIN) == 1)
    {
      Serial.print(".");
      delay(1000);
    }
    firstLoop = false;

    // Ensure we waited at least MIN_ACTIVE_TIME before looping around
    while ((millis() - startMillis) < MIN_ACTIVE_TIME)
      delay(100);
  }
  else
  {
    root["motion"] = "inactive";
    root["battery"] = vcc;
    root.printTo(stringBuffer);
    hubClient.send(stringBuffer);
    hubClient.run();
    yield();

    Serial.printf("Sent: %s\r\n", stringBuffer);

    // Shut down wifi and wait for everything to disconnect
    WiFi.disconnect(true);
    delay(500);

    // Turn off by bringing CH_PD low
    digitalWrite(HOLD_PIN, LOW);
    yield();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Helper Functions (forward declarations above)
////////////////////////////////////////////////////////////////////////////////

static void messageCallout(String message)
{
  Serial.print("Received hub message: '");
  Serial.print(message);
  Serial.println("' ");
}

static float getVccPercent()
{
  // Since the charging module cuts off at 2.5V, and the vreg will drop anything
  // above 3.3 to 3.3, I will assume that > 3.2V means full, 3.2V will be reported
  // as 50% and 2.5V wil be reported at 0%, using linear scaling across that range.
  // Anything above 3.2% will be reported as 100%. (Note: if you are using an ESP8266
  // that has the ADC pin exposed for your use, you could get a real VBAT percentage
  // by using a voltage divider to get it into a range the ADC can accept, and just
  // return that value.)
  //
  // Scaling equation used:
  // f(x) = C*(1-(x-A)/(B-A)) + D*((x-A)/(B-A))
  //
  // Where [A:B] is [2.5:3.2] and [C:D] is [0.0:50.0]

  uint16_t x = ESP.getVcc();
  uint16_t A = 2500, B = 3200;
  float C = 0.0, D = 50.0;
  float result = 100.0;
  if (x <= B)
  {
    result = C * (1 - (x - A) / (B - A)) + D * ((x - A) / (B - A));
  }

  return result;
}