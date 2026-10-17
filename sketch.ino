#include <WiFi.h>
#include <HTTPClient.h>
#include "DHTesp.h"
#include "ThingSpeak.h"

const int DHT_PIN = 15;

DHTesp dhtSensor;

const char* WIFI_NAME = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

unsigned long myChannelNumber = 3500855;
const char* myApiKey = "UJJB0HJH5MMLPI3Q";
const char* alertApiKey = "TAKDQutW8JSpuaZefss";

WiFiClient client;


// ---------- SEND EMAIL ----------
void sendEmail(String subject, String message) {

  HTTPClient http;

  http.begin("https://api.thingspeak.com/alerts/send");

  http.addHeader("ThingSpeak-Alerts-API-Key", alertApiKey);
  http.addHeader("Content-Type", "application/json");

  String jsonData = "{\"subject\":\"" + subject +
                    "\",\"body\":\"" + message + "\"}";

  int response = http.POST(jsonData);

  Serial.print("Email response: ");
  Serial.println(response);

  if (response == 202) {
    Serial.println("Email alert sent successfully!");
  } else {
    Serial.println("Email alert failed!");
  }

  http.end();
}


// ---------- SETUP ----------
void setup() {

  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);

  Serial.println("ThingSpeak started!");
}


// ---------- LOOP ----------
void loop() {

  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  if (isnan(data.temperature) || isnan(data.humidity)) {

    Serial.println("DHT22 reading failed!");

    delay(2000);

    return;
  }


  // Display sensor values

  Serial.print("Temperature: ");
  Serial.print(data.temperature, 2);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(data.humidity, 1);
  Serial.println(" %");


  // Upload to ThingSpeak

  ThingSpeak.setField(1, data.temperature);
  ThingSpeak.setField(2, data.humidity);

  int response = ThingSpeak.writeFields(
    myChannelNumber,
    myApiKey
  );

  if (response == 200) {
    Serial.println("Data uploaded successfully!");
  } else {
    Serial.print("ThingSpeak upload failed: ");
    Serial.println(response);
  }

  // ---------- EMAIL ALERTS ----------

  if (data.temperature > 35) {

    sendEmail(
      "Temperature Alert",
      "Temperature has exceeded 35 C."
    );

  }


  if (data.humidity > 80) {

    sendEmail(
      "Humidity Alert",
      "Humidity has exceeded 80 percent."
    );
  }

  Serial.println("-------------------------");

  delay(15000);
}