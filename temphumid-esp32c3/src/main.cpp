#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <HTTPClient.h>

// SENSOR CONFIGURATION
#define DHTPIN D5
#define DHTTYPE DHT11

// WIFI CONFIGURATION
const char *ssid = "Haystack Labs";
const char *password = "Labs2024";
const char *serverUrl = "labs.local";

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    String jsonPayload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0)
    {
      Serial.println("HTTP Response code: " + String(httpResponseCode));
    }
    else
    {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }
    http.end();
  }
  delay(1000 * 60); // 1 minute
}
