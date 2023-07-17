#include "Arduino.h"
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "DHT.h"

// Change if necessary
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

// Change if necessary
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

// Declare variable names
float h;   
float t;
float ph;
float hic;

#define SensorPin 2
#define Offset 0.00
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLength  40

int pHArray[ArrayLength];   // Store the average value of the sensor feedback
int pHArrayIndex = 0;

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

/**
 * Establishes a connection to the AWS IoT platform.
 * Configures WiFi and sets up the MQTT connection.
 */
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint 
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IoT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}

/**
 * Publishes sensor data to the AWS IoT platform.
 * Creates a JSON document and publishes it as a message.
 */
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["humidity"] = h;
  doc["temperature"] = t;
  doc["pH"] = ph;
  doc["HeatIndex"] = hic;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

/**
 * Handles incoming MQTT messages.
 * Deserializes the JSON message and prints the content.
 */
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

/**
 * Calculates the average value of an array.
 * If the number of values is less than 5, calculates directly the statistics.
 * Otherwise, uses a more optimized algorithm.
 */
double avergearray(int* arr, int number)
{
  int i;
  int max, min;
  double avg;
  long amount = 0;
 
  if (number <= 0)
  {
    Serial.println("Error: Number for the array to be averaged is invalid!");
    return 0;
  }
 
  if (number < 5)   // If the number of values is less than 5, calculate directly the statistics
  {
    for (i = 0; i < number; i++)
    {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  }
  else
  {
    if (arr[0] < arr[1])
    {
      min = arr[0];
      max = arr[1];
    }
    else
    {
      min = arr[1];
      max = arr[0];
    }
    for (i = 2; i < number; i++)
    {
      if (arr[i] < min)
      {
        amount += min;        // arr < min
        min = arr[i];
      }
      else
      {
        if (arr[i] > max)
        {
          amount += max;    // arr > max
          max = arr[i];
        }
        else
        {
          amount += arr[i]; // min <= arr <= max
        }
      }
    }
    avg = (double)amount / (number - 2);
  }
  return avg;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Aqua Sensor starting");
  connectAWS();
  dht.begin();
}

void loop()
{ 
  // Wait a few seconds between measurements.
  delay(2000);

  // Read temperature , humidity and heatindex 
  h = dht.readHumidity();
  t = dht.readTemperature();
  hic = dht.computeHeatIndex(t, h, false);

  if (isnan(h) || isnan(t) || isnan(hic))  // Check if any reads failed and exit early (to try again).
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C  Heat index:  "));
  Serial.print(hic);
  Serial.println(F("°C "));

  // Read pH Sensor
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
    if (pHArrayIndex == ArrayLength) pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLength) * 5.0 / 1024;
    pHValue = 3.5 * voltage + Offset;
    samplingTime = millis();
  }

  if (millis() - printTime > printInterval)   // Every 800 milliseconds, print a numerical value and convert the state of the LED indicator
  {
    Serial.print("Voltage:");
    Serial.print(voltage, 2);
    Serial.print("    pH value: ");
    Serial.println(pHValue, 2);
    digitalWrite(LED, digitalRead(LED) ^ 1);
    printTime = millis();
  }

  publishMessage();
  client.loop();
  delay(1000);
}
