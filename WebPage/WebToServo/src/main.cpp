#include "Arduino.h"
#include <WiFi.h>
#include "esp_camera.h"
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <ESPAsyncWebServer.h>
#include "secrets.h"

#define CAM_ON 0
#define FEEDFISH 0

#if CAM_ON
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#endif

//const char* ssid = ;
//const char* password = ;
//const char* serverUrl = "http://:5000/receive_video"; // Update with your server IP

unsigned long previousMillis = 0;
const long interval = 1000;
bool operationComplete =false;

/* HTTP Stuff*/

AsyncWebServer server(80);

void handleFeed(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Feeding fish now");
}


/*Servo stuff*/

Servo myservo;
int pos = 0;

#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
int servoPin = 14;
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
int servoPin = 7;
#else
int servoPin = 18;
#endif


# define FEEDINGTIME 5000

void handleRequest() {
  operationComplete = true;
  Serial.println("handledReqeust");
}

void feedFish() {
  // Function to feed the fish based on specified amount of time
  myservo.attach(servoPin, 1000, 2000);
  Serial.println("RUNNING");

  for (int angle = 0; angle <=  180; angle += 3) { // goes from 0 degrees to 1000 degrees
		// in steps of 1 degree
		myservo.write(angle);    // tell servo to go to position in variable 'pos'
		delay(4);             // waits 15ms for the servo to reach the position

  }

    for (int angle = 180; angle >=  0; angle -= 3) { // goes from 0 degrees to 1000 degrees
		// in steps of 1 degree
		myservo.write(angle);    // tell servo to go to position in variable 'pos'
		delay(4);             // waits 15ms for the servo to reach the position

  }

  // Stop the servo after the specified time
  //myservo.write(pos); // Move the servo to the center position (or any desired default position)
  //operationComplete = true; // Set the flag to indicate that the operation is complete
  myservo.detach();
}

#if CAM_ON
/* Camera Initalisation functions */
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // or PIXFORMAT_RGB565 for raw image format
  
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
#endif
void setup() {
  // Initialization code for camera and Wi-Fi
  Serial.begin(9600);
  // Connect to Wi-Fi
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  Serial.println(psramInit());

  // Initialize the servo

  myservo.setPeriodHertz(50);    // standard 50 hz servo
	

  //Routing to handle HTTP requests
  server.on("/feed", HTTP_POST, [](AsyncWebServerRequest *request){
    handleRequest();
  });

  server.begin();
  //xTaskCreatePinnedToCore(feedFish, "feedFishTask", 4096, NULL, 1, NULL, 0);

//  #if CAM_ON
//  // Camera configuration
//  initCamera();
//
//  // Start streaming video frames to the server
//  while (true) {
//    camera_fb_t *fb = esp_camera_fb_get();
//    if (!fb) {
//      Serial.println("Camera capture failed");
//      delay(1000);
//      continue;
//    }
//
//    HTTPClient http;
//    http.begin(serverUrl); // Use HTTPClient to send the video stream data
//    http.addHeader("Content-Type", "application/octet-stream");
//    int httpCode = http.POST((uint8_t *)fb->buf, fb->len); // Send the video stream as raw data
//
//    if (httpCode > 0) {
//      // Successful response from the server
//      // Handle the response if needed
//    } else {
//      Serial.printf("HTTP request failed with error code %d\n", httpCode);
//    }
//
//    esp_camera_fb_return(fb);
//    delay(100); // Change the delay as needed
//  }
//  #endif
}

void loop() {
  // Check if the operation is complete, if not, it will continue in the next iteration
  if (operationComplete) {
    Serial.println("OI");
    feedFish();
    operationComplete = false;
  }

}

