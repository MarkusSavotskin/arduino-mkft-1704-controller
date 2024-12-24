#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>

// WiFi configuration
const char* ssid = "ssid";
const char* password = "password";

// MQTT configuration
const char* mqtt_server = "address";
const char* mqtt_user = "user";
const char* mqtt_password = "password";
const char* spotlight_topic = "home/esp8266/spotlight";

const int buttonPin = 0;  

WiFiClient espClient;
PubSubClient client(espClient);

// Non-blocking WiFi initialization
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Attempting to connect to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

// Reconnect WiFi if disconnected
void ensureWiFiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("Reconnecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nReconnected to WiFi!");
  }
}

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println(message);

  // Check if the message is from the spotlight topic
  if (String(topic) == spotlight_topic) {
    if (message == "on") {
      handleOnCommand();  // Handle "on" command
    } else if (message == "off") {
      handleOffCommand();  // Handle "off" command
    }
  }
}

// Reconnect MQTT if disconnected
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(spotlight_topic);  // Subscribe to the spotlight topic
      Serial.print("Subscribed to topic: ");
      Serial.println(spotlight_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // Wait before retrying
    }
  }
}

// Function to simulate a button press
void simulateButtonPress() {
  digitalWrite(buttonPin, LOW);
  delay(200);
  digitalWrite(buttonPin, HIGH);
}

// Function to handle "on" command (single press)
void handleOnCommand() {
  Serial.println("Handling ON command");
  simulateButtonPress();
}

// Function to handle "off" command (double press)
void handleOffCommand() {
  Serial.println("Handling OFF command");
  simulateButtonPress();
  delay(500);
  simulateButtonPress();
}

void setup() {
  pinMode(buttonPin, OUTPUT);
  digitalWrite(buttonPin, HIGH);
  Serial.begin(9600);

  initWiFi();

  // Initialize MQTT client
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Ensure WiFi is connected
  ensureWiFiConnected();

  // Ensure MQTT is connected
  if (!client.connected()) {
    reconnectMQTT();
  }
  
  // Process any incoming MQTT messages
  client.loop();
}
