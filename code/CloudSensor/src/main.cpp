// Library
#include "Arduino.h"
#include "Secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Constants
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
#define DHTTYPE DHT11   // DHT 11
#define OFFSET 0.00
#define SAMPLING_INTERVAL 3000 //600000 ~ 10 MINS
#define ARRAYLENGTH 40
#define RL 47  // The value of resistor RL is 47K
#define m -0.263 // Enter calculated Slope
#define b 0.42 // Enter calculated intercept
#define Ro 20 // Enter found Ro value


// PINS
#define DHTPIN 4       //  DHT11 sensor
#define ONE_WIRE_BUS 3 //  DS18B20 temperature sensor
#define SensorPin 2    //  SEN0161 pH sensor
#define PIN_MQ135 1    // MQ135 Analog Input Pin

// Global variables
float humidity;
float temperature;
float pHValue;
float heatIndex;
float waterTempC;
float ammonia_ppm;
unsigned long lastReadTime = 0;
const int ledPin = 23;

//Random Generation 
float baselineAmmoniaValue = 0;
float baselineWaterTempValue = 0;
float baselinepHValue = 0;
const float spikeProbability = 0.05;    // Probability of a spike occurring (adjust as needed)
const float spikeIntensity = 1.5;     // Maximum value of the spike (adjust as needed)

// Create the lcd object address 0x3F and 16 columns x 2 rows 
LiquidCrystal_I2C lcd (0x27, 16,2);  
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0; 
const int daylightOffset_sec = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

// Function declarations
void display(float humidity, float temperature, float heatIndex, float pH , float waterTempC , float ammonia_ppm);
float getpHValue();
float getWaterTempValue();
float getAmmoniaValue();
double avergearray(int* arr, int number);
void connectAWS();
void publishMessage();
void messageHandler(char* topic, byte* payload, unsigned int length);
unsigned long getCurrentUnixTimestamp();


void setup()
{
  Serial.begin(115200);
  connectAWS();     //  Connect to AWS
  dht.begin();        // Initialize the DHT11 sensor
  sensors.begin();    // Initialize DS18B20 sensor
  lcd.init();         // Initialize the LCD connected 
  lcd.backlight ();   // Turn on the backlight on LCD. 
  //Wire.begin();
  delay(1000);
  Serial.println("Aqua Sensor starting");
  delay(3000);
  lcd.clear();
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
    ammonia_ppm = getAmmoniaValue() ;

    // Display on Monitor
    display(humidity, temperature, heatIndex, pHValue ,waterTempC , ammonia_ppm);

    // Update the last pH reading time
    lastReadTime = millis();
  }
  // Transfer data to cloud
    publishMessage();
    client.loop();
}


/***************** Display  *********************/

void display(float humidity, float temperature, float heatIndex, float pH , float waterTempC , float ammonia_ppm)
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

  else if (isnan(ammonia_ppm))
  {
    Serial.println(F("Failed to read from Ammonia sensor!"));
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
  Serial.print(pH);
  Serial.print(F("  Ammonia : "));
  Serial.print(ammonia_ppm);
  Serial.println("ppm");

  // Clear the LCD display
  lcd.clear();

  // first flash
  lcd.setCursor(0, 0);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Temperature: ");
  lcd.print(temperature);
  lcd.print("C");

  delay(3000);
  lcd.clear();

  // Second flash
  lcd.setCursor(0, 0);
  lcd.print("Heat Index: ");
  lcd.print(heatIndex);
  lcd.print(" C ");

  lcd.setCursor(0, 1);
  lcd.print("pH: ");
  lcd.print(pH);

  delay(3000);
  lcd.clear();

  // Third flash
  lcd.setCursor(0, 0);
  lcd.print("Water Temp: ");
  lcd.print(waterTempC);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Ammonia (ppm): ");
  lcd.print(ammonia_ppm);
  lcd.print("ppm");

  delay(3000);
  lcd.clear();

}

/**************** Ammonia Sensor  *****************/
float getAmmoniaValue() 
{
  float ppm;
  float VRL; // Voltage drop across the MQ sensor
  float Rsg; // Sensor resistance at gas concentration
  float ratio; // Define variable for ratio

  VRL = analogRead(PIN_MQ135) * (5.0 / 1023.0); // Measure the voltage drop and convert to 0-5V
  Rsg = ((5.0 * RL) / VRL) - RL; // Use the formula to get Rsg value
  ratio = Rs / Ro; // Find the ratio Rs/Ro

  ppm = pow(10, ((log10(ratio) - b) / m)); // Use the formula to calculate ppm

  return ppm; // Return ammonia concentration in ppm
}

/************ Water temperature Sensor ************/
float getWaterTempValue()
{ 
  float waterTempC ;

  sensors.requestTemperatures(); // Send the command to get temperatures
  waterTempC = sensors.getTempCByIndex(0);

  return waterTempC;
}

/***************** pH Sensor  *********************/

float getpHValue()
{ 
  float pHValue; // Declare pHValue outside the #ifdef block

  // Code related to SEN0161 sensor
  static int pHArray[ARRAYLENGTH];
  static int pHArrayIndex = 0;
  static float voltage;

  pHArray[pHArrayIndex++] = analogRead(SensorPin);
  if (pHArrayIndex >= ARRAYLENGTH) pHArrayIndex = 0;
  voltage = avergearray(pHArray, ARRAYLENGTH) * 5.0 / 1024;
  pHValue = 3.5 * voltage + OFFSET; 


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
  doc["device_ID"] = "cloudSensor_01";
  doc["room"] = "roof_top_garden";
  doc["time"] = getCurrentUnixTimestamp();
  doc["humidity"] = humidity;
  doc["temperature"] = temperature;
  doc["pH"] = pHValue;
  doc["HeatIndex"] = heatIndex;
  doc["waterTempC"] = waterTempC;
  doc["Ammonia"] = ammonia_ppm;
  
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

/***************** Get time  *********************/

unsigned long getCurrentUnixTimestamp() {
  // Update the NTP client to get the current time
  timeClient.update();

  // Get the current Unix timestamp (in seconds)
  return timeClient.getEpochTime();
}