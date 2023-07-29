#include <Arduino.h>
#include "WiFi.h"


// put function declarations here:
const char* ssid = "";
const char* password = "";

void setup() {
  // put your setup code here, to run once:
  //int result = myFunction(2, 3);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  // Print local IP address and start web server
  //Serial.println("");
  //Serial.println("WiFi connected.");
  //Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
}
