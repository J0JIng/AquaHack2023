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
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Constants
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
#define DHTTYPE DHT11   // DHT 11
#define OFFSET 0.00
#define SAMPLING_INTERVAL 20
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


// SENSOR
//#define SEN0161_CONNECTED 
//#define DHT11_CONNECTED 

//#define DS18B20_CONNECTED  
//#define MQ135_CONNECTED 

// Global variables
float humidity;
float temperature;
float pHValue;
float heatIndex;
float waterTempC;
float ammonia_ppm;
unsigned long lastReadTime = 0;

// Create the lcd object address 0x3F and 16 columns x 2 rows 
LiquidCrystal_I2C lcd (0x27, 16,2);  
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Function declarations
void display(float humidity, float temperature, float heatIndex, float pH , float waterTempC , float ammonia_ppm);
float getpHValue();
float getWaterTempValue();
float getAmmoniaValue();
double avergearray(int* arr, int number);
void connectAWS();
void publishMessage();
void messageHandler(char* topic, byte* payload, unsigned int length);

void setup()
{
  Serial.begin(115200);
  //connectAWS(); //  Connect to AWS
  dht.begin(); // Initialize the DHT11 sensor
  sensors.begin(); // Initialize DS18B20 sensor
  lcd.init();  // Initialize the LCD connected 
  lcd.backlight (); // Turn on the backlight on LCD. 
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
    #ifdef DHT11_CONNECTED 
    
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    
    #else

    humidity = random(10, 15); 
    temperature = random(20, 25);
    heatIndex = random(20, 25);

    #endif 
    
    pHValue = getpHValue();
    waterTempC = getWaterTempValue();
    ammonia_ppm = getAmmoniaValue() ;

    // Display on Monitor
    display(humidity, temperature, heatIndex, pHValue ,waterTempC , ammonia_ppm);

    // Transfer data to cloud
    //publishMessage();
    //client.loop();

    // Update the last pH reading time
    lastReadTime = millis();
  }
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

#ifdef MQ135_CONNECTED 

  float VRL; // Voltage drop across the MQ sensor
  float Rs; // Sensor resistance at gas concentration
  float ratio; // Define variable for ratio

  VRL = analogRead(MQ_sensor) * (5.0 / 1023.0); // Measure the voltage drop and convert to 0-5V
  Rs = ((5.0 * RL) / VRL) - RL; // Use the formula to get Rs value
  ratio = Rs / Ro; // Find the ratio Rs/Ro

  ppm = pow(10, ((log10(ratio) - b) / m)); // Use the formula to calculate ppm

#else 

  int randomValue = random(10, 20); // Generate a random value between 10 and 20
  ppm = randomValue / 100.0; // Scale the integer to a floating-point value between 0.1 and 0.2

#endif

  return ppm; // Return ammonia concentration in ppm
}

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
