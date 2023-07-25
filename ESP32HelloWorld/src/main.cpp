// Library
#include "Arduino.h"
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <MQ135.h>

// Constants
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
#define DHTTYPE DHT11   // DHT 11
#define OFFSET 0.00
#define SAMPLING_INTERVAL 20
#define ARRAYLENGTH 40

// PINS
#define DHTPIN 4       //  DHT11 sensor
#define ONE_WIRE_BUS 3 //  DS18B20 temperature sensor
#define SensorPin 2    //  SEN0161 pH sensor
#define PIN_MQ135 1   // MQ135 Analog Input Pin

// SENSOR
#define SEN0161_CONNECTED 
#define DHT11_CONNECTED 

//#define DS18B20_CONNECTED  
//#define MQ135_CONNECTED 

// Global variables
float humidity;
float temperature;
float pHValue;
float heatIndex;
float waterTempC;
unsigned long lastReadTime = 0;

//MQ135 mq135_sensor(PIN_MQ135);
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Function declarations
void display(float humidity, float temperature, float heatIndex, float pH , float waterTempC);
float getpHValue();
float getWaterTempValue();
double avergearray(int* arr, int number);
void connectAWS();
void publishMessage();
void messageHandler(char* topic, byte* payload, unsigned int length);

void setup()
{
  Serial.begin(115200);
  Serial.println("Aqua Sensor starting");
  connectAWS();
  dht.begin();
  sensors.begin();
}

void loop()
{
  // Wait for xx minutes to elapse since the last reading
  if (millis() - lastReadTime >= SAMPLING_INTERVAL)
  {
    // Reading variables
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    pHValue = getpHValue();
    waterTempC = getWaterTempValue();

    // Display on Monitor
    display(humidity, temperature, heatIndex, pHValue ,waterTempC);

    // Transfer data to cloud
    publishMessage();
    client.loop();

    // Update the last pH reading time
    lastReadTime = millis();
  }
}



/***************** Display  *********************/

void display(float humidity, float temperature, float heatIndex, float pH , float waterTempC)
{
  if (isnan(humidity) || isnan(temperature) || isnan(heatIndex))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  else if (isnan(pH))
  {
    Serial.println(F("Failed to read from pH sensor!"));
    return;
  }
  else if (isnan(waterTempC))
  {
    Serial.println(F("Failed to read from Water Temperature sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  AmbientTemperature: "));
  Serial.print(temperature);
  Serial.print(F("°C  WaterTemperature: "));
  Serial.print(waterTempC);
  Serial.print(F("°C  Heat index:  "));
  Serial.print(heatIndex);
  Serial.print(F("°C  pH: "));
  Serial.println(pH);
}

/**************** Ammonia Sensor  *****************/


/************ Water temperature Sensor ************/
float getWaterTempValue()
{ 
  float waterTempC ;

#ifdef DS18B20_CONNECTED 
  sensors.requestTemperatures(); // Send the command to get temperatures
  waterTempC = sensors.getTempCByIndex(0);
  
#else
  int randomValue = random(250, 270); // Generate a random value between 250 and 269
  waterTempC = randomValue / 10.0; // Scale the integer to a floating-point value between 25.0 and 26.9

#endif
  return waterTempC;
}

/***************** pH Sensor  *********************/

float getpHValue()
{ 
  float pHValue; // Declare pHValue outside the #ifdef block

#ifdef SEN0161_CONNECTED 
  // Code related to SEN0161 sensor
  static int pHArray[ARRAYLENGTH];
  static int pHArrayIndex = 0;
  static float voltage;

  pHArray[pHArrayIndex++] = analogRead(SensorPin);
  if (pHArrayIndex >= ARRAYLENGTH) pHArrayIndex = 0;
  voltage = avergearray(pHArray, ARRAYLENGTH) * 5.0 / 1024;
  pHValue = 3.5 * voltage + OFFSET; // Update the pHValue inside the #ifdef block

#else
  // Generate random pH values
  int randomValue = random(67, 73); // Generate a random integer between 0 and 140
  pHValue = randomValue / 10.0; // Scale the integer to a floating-point pH value between 0 and 14

#endif

  return pHValue;
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


/***************** AWS API *********************/

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
  doc["humidity"] = humidity;
  doc["temperature"] = temperature;
  doc["pH"] = pHValue;
  doc["HeatIndex"] = heatIndex;
  doc["waterTempC"] = waterTempC;
  
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
