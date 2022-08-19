#include "WiFi.h"
#include <Arduino.h>
#include <EasyButton.h>
#include <PubSubClient.h>
#include <Wire.h>

const char *ssid = "Lima";
const char *password = "Limesaregreat";

const char *mqtt_server = "10.0.0.2";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


// Button stuff
int buttonPushCounter = 0;
bool autoChangePatterns = false;

#define STROBE_BTN_PIN 5

int debounce = 40;

bool pullup = true;

bool invert = true;

EasyButton strobeButton(STROBE_BTN_PIN);
uint8_t strobeStatus = 0;

void toggleStrobe();
void changePalette();
void reconnect();
void mqttCallback(char *topic, byte *message, unsigned int length);

void setup() {
  
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
  strobeButton.begin();
  strobeButton.onPressed(toggleStrobe);
  strobeButton.onPressedFor(1000, changePalette);
  // modeBtn.onSequence(3, 2000, changePalette);
  // modeBtn.onSequence(5, 2000, brightnessOff);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("StrobeButton")) {
      Serial.println("Connected to MQTT Server");
      Serial.println("Subscribing to topics");
      // Subscribe
      client.subscribe("strobe");
      Serial.println("Subscribed to topics");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttCallback(char *topicIn, byte *message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topicIn);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Changes the output state according to the message
  String topic = String(topicIn);
  
  if (topic == "strobe") {
    if (messageTemp == "true" || messageTemp == "on") {
      strobeStatus = 1;
    } else {
      strobeStatus = 0;
    }
  }
  else {
    Serial.println("Received unknown message, ignoring");
  }
}

void toggleStrobe() {
  Serial.println("Toggling strobe");
  Serial.println("Strobe status:" + strobeStatus);
  if(strobeStatus == 0) {
    strobeStatus = 1;
    client.publish("strobe", "on");
  } else {
    strobeStatus = 0;
    client.publish("strobe", "off");
  }
}

void changePalette() {
  Serial.println("Changing palette");
  client.publish("changePalette", "");
}

/********************************
                   MAIN LOOP
**************************************/
void loop() {

  if (!client.connected()) {
    Serial.println("Not connected anymore");
    reconnect();
  }
  client.loop();
  strobeButton.read();

}