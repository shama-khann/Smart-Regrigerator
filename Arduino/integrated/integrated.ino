#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include "HX711.h" // Install from Library Manager


#define FIREBASE_HOST "esp8266-8bd59-default-rtdb.firebaseio.com" // Replace with your Firebase project ID
#define FIREBASE_AUTH "AIzaSyDbMZvryszhUJhioysDE0E3C5lS13hDw_E" // Replace with your Firebase API key
#define WIFI_SSID "D-Link" // Replace with your Wi-Fi SSID
#define WIFI_PASSWORD "rjc3304011" // Replace with your Wi-Fi password
#define DOOR_SENSOR_PIN D1 // Replace with the pin connected to your door sensor
#define CLOCK_PIN D5 // HX711 clock pin
#define DATA_PIN D6 // HX711 data pin
#define SENSOR_PIN A0 // MQ-4 sensor analog pin
#define GAS_THRESHOLD  // Adjust based on your testing
#define CALIBRATION_FACTOR -471880.00 // Adjust to your calibration factor


FirebaseData firebaseData;
HX711 scale;
int flags[3] = {0, 0, 0}; // Initialize with zeros
 // Track state changes for each sensor: door, load, gas


void setup() {
  Serial.begin(9600);
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(CLOCK_PIN, INPUT);
  pinMode(DATA_PIN, INPUT);
  scale.begin(DATA_PIN, CLOCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare(); // Reset the scale to 0


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to WiFi with IP: ");
  Serial.println(WiFi.localIP());


  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}


void loop() {
  // Check door sensor
  if (digitalRead(DOOR_SENSOR_PIN) == HIGH && flags[0] == 0) {
    Serial.println("open");
    doorNotification("open"); // Send notification to Firebase
    flags[0] = 1;
  } else if (digitalRead(DOOR_SENSOR_PIN) == LOW && flags[0] == 1) {
     Serial.println("close");
    doorNotification("close");
    flags[0] = 0;
  }


  // Check load sensor
  float weight = scale.get_units(5);
  if (weight < 1.00 && flags[1] == 0) {
    Serial.println("empty");
    weightNotification("empty"); // Send notification to Firebase
    flags[1] = 1;
  } else if (weight >= 1.00 && flags[1] == 1) {
    weightNotification("filled");
    flags[1] = 0;
  }
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" KG");


  // Check gas sensor
  int sensorValue = analogRead(SENSOR_PIN);
  if (sensorValue > GAS_THRESHOLD && flags[2] == 0) {
    Serial.println("rotten");
   gasNotification("rotten"); // Send notification to Firebase
    flags[2] = 1;
  } else if (sensorValue <= GAS_THRESHOLD && flags[2] == 1) {
    Serial.println("Fresh!");
    gasNotification("Fresh!");
    flags[2] = 0;
  }
  Serial.print("Sensor value: ");
  Serial.println(sensorValue);


  delay(3000); // Add a delay to avoid sending too many notifications in rapid succession
}


void doorNotification(const String& message) {
  if (Firebase.setString(firebaseData, "/door", message)) {
    Serial.println("");
  } else {
    Serial.print("Firebase error: ");
    Serial.println(firebaseData.errorReason());
  }
}


void weightNotification(const String& message) {
  if (Firebase.setString(firebaseData, "/weight", message)) {
    Serial.println("");
  } else {
    Serial.print("Firebase error: ");
    Serial.println(firebaseData.errorReason());
  }
}


void gasNotification(const String& message) {
  if (Firebase.setString(firebaseData, "/gas", message)) {
    Serial.println("");
  } else {
    Serial.print("Firebase error: ");
    Serial.println(firebaseData.errorReason());
  }
}