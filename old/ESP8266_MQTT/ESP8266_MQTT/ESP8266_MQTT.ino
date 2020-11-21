/*
 Name:		ESP8266_MQTT.ino
 Created:	7/7/2018 1:34:18 PM
 Author:	hazao
*/

// the setup function runs once when you press reset or power the board
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#define NUM_LEDS 8
#define PIN 12		// Pin numbers for esp8266 are the same as the GPIO numbers, i.e gpio12 = PIN 12
#define BRIGHTNESS 255
int wait = 10;
const char* ssid = "BTHub5-Q8PC";
const char* password = "Energy2017";
const char* mqtt_server = "io.adafruit.com";
const char* mqttUsername = "HarryOMalley";
const char* mqttPassword = "78b91cccb9054734a0009f4ab4e01dc0";
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
WiFiClient espClient;
PubSubClient client(espClient);
uint32_t currentColour, white = strip.Color(255, 255, 255);
uint32_t red = strip.Color(255, 0, 0), green = strip.Color(0, 255, 0), blue = strip.Color(0, 0, 255), purple = strip.Color(255, 0, 255), cyan = strip.Color(0, 255, 255), yellow = strip.Color(255, 255, 0), gold = strip.Color(255, 200, 0);
int inRainbow = 0;


/*
	When sending the MQTT messages, append a - at the end of the message
	so that we can check when the message is over, avoiding extraneous characters
*/





void setup()
{
	Serial.begin(115200);
	delay(1000);

	// We start by setting the mqtt server
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);

	Serial.println("Starting Leds");
	strip.begin();
	strip.setBrightness(BRIGHTNESS);
	for (int n = 0; n < NUM_LEDS; n++)
	{
		strip.setPixelColor(n, 255, 255, 255);

	}
	//strip.show();

	Serial.println();
	Serial.println();
	Serial.print("Set ssid to ");
	Serial.println(ssid);
	Serial.print("Set mqtt server to ");
	Serial.println(mqtt_server);
}

void maintainMQTT()
{
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
}

int value = 0;
// the loop function runs over and over again until power down or reset
void loop()
{
	maintainMQTT();
}


/*
	This function takes the input that is recieved from MQTT and uses it to execute the corresponding program
*/
void programs(int program)
{
	switch (program)
	{
	case 0:
		Serial.println("going to number 0 ");
		leds(0);
		break;
	case 1:
		Serial.println("going to number 1 ");
		leds(1);
		break;
	case 2:
		Serial.println("going to white ");
		setColour(white);
		break;
	case 3:
		Serial.println("going to rainbow ");
		rainbow(wait);
		break;
	case 4:
		Serial.println("going to random rainbow ");
		rainbowCycle(wait);
		break;
	default:
		Serial.print("Cannot execute program: ");
		Serial.println(program);
		break;
	}
}


//int checkForNumbers(byte* payload, int length)
//{
//	char number[10];
//	// loop through the payload, extracting characters until the end of message character is found
//	for (int i = 0; i < length; i++)
//	{
//		if (char(payload[i]) == char("-"))
//		{
//			Serial.println("End of the message");
//			break;
//		}
//		number[i] = (char)payload[i];
//	}
//	int finalNumber = atoi(number);
//	Serial.println(finalNumber);
//	Serial.println(number);
//	return finalNumber;
//
//	// old way
////	if (isdigit(payload[2]))
////	{
////		for (int i = 0; i < 3; i++)
////		{
////			number[i] = (char)payload[i];
////		}
////		int finalNumber = atoi(number);
////		Serial.println(finalNumber);
////		Serial.println(number);
////		makeColour(int(number));
////	}
////	else if (isdigit(payload[1]))
////	{
////		for (int i = 0; i < 2; i++)
////		{
////			number[i] = (char)payload[i];
////		}
////		int finalNumber = atoi(number);
////		Serial.println(finalNumber);
////		Serial.println(number);
////		makeColour(int(number));
////	}
//}


void makeColour(int colour)
{
	if (colour > 255)
		colour = colour / 10;
	uint32_t newColour = Wheel(colour);
	setColour(newColour);
}


void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");

	// check what number the message is
	for (int i = 0; i < length; i++)
	{
		//if (isdigit(payload[2]))
		//{
		//	for (int i = 0; i < 3; i++)
		//	{
		//		number[i] = (char)payload[i];
		//	}
		//	int finalNumber = atoi(number);
		//	Serial.println(finalNumber);
		//	Serial.println(number);
		//	makeColour(int(number));
		//}
		//else if (isdigit(payload[1]))
		//{
		//	for (int i = 0; i < 2; i++)
		//	{
		//		number[i] = (char)payload[i];
		//	}
		//	int finalNumber = atoi(number);
		//	Serial.println(finalNumber);
		//	Serial.println(number);
		//	makeColour(int(number));
		//}
		//else
		//{
		char receivedChar = (char)payload[i];
		Serial.println(receivedChar);
		//Serial.println(payload);
		if (receivedChar == '0') // turn leds off
			programs(0);
		if (receivedChar == '1') // turn leds on
			programs(1);
		if (receivedChar == '2') // turn leds white
			programs(2);
		if (receivedChar == '3') // turn leds rainbow
			programs(3);
		if (receivedChar == '4') //
			programs(4);
		//}
		//else
		//{
		//	for (int i = 0; i < 3; i++)
		//	{
		//		if (isdigit(payload))
		//		number[i] = (char)payload[i];
		//	}
		//	int finalNumber = int(number);
		//	Serial.println(finalNumber);
		//	Serial.println(number);
		//	makeColour(int(number));
		//}
	}
}


void reconnect()
{
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("ESP8266 Client", mqttUsername, mqttPassword)) {
			Serial.println("connected");
			// ... and subscribe to topic
			client.subscribe("HarryOMalley/feeds/esp");
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void leds(int onOff)
{
	if (onOff == 0) // off
	{
		Serial.println("turning off");
		strip.setBrightness(0);
	}
	else if (onOff == 1) // on
	{
		Serial.println("turning on");
		strip.setBrightness(BRIGHTNESS);
	}
}

void setColour(uint32_t colour)
{
	while (true)
	{
		for (int n = 0; n < NUM_LEDS; n++)
		{
			strip.setPixelColor(n, colour);
			strip.show();
		}
		maintainMQTT();
		strip.show();
	}
}


int rainbowCycle(uint8_t wait)
{
	inRainbow = 2;
	uint16_t i, j;
	uint16_t randnum[NUM_LEDS];
	for (i = 0; i < NUM_LEDS; i++)
	{
		randnum[i] = random(256);
	}
	while (true)
	{
		for (j = 0; j < 256 * 100; j++)
		{ // 5 cycles of all colors on wheel - i think this just makes it do it over and over
			for (i = 0; i < NUM_LEDS; i++)
			{
				strip.setPixelColor(i, Wheel(((randnum[i] * 256 / NUM_LEDS) + j) & 255));
			}

			maintainMQTT();
			strip.show();
			delay(wait);
		}
	}
}

int rainbow(uint8_t wait)
{
	inRainbow = 1;
	uint16_t i, j;
	while (true)
	{
		for (j = 0; j < 256; j++) {
			for (i = 0; i < strip.numPixels(); i++) {
				strip.setPixelColor(i, Wheel((i + j) & 255));
			}
			maintainMQTT();
			strip.show();
			delay(wait);
		}
	}
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if (WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}