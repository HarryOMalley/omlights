#include "Functions.h"
#include <Adafruit_NeoPixel.h>
#define PIN 7
#define NUM_LEDS 300
char inString[20], inChar, exitString[] = "exit";


String programString;
int colour[3];

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);


void setup()
{

	Serial.begin(9600);
	Serial.println("Starting...");
	strip.begin();
	strip.setBrightness(255);
	uint32_t red = strip.Color(255, 0, 0), green = strip.Color(0, 255, 0), blue = strip.Color(0, 0, 255), purple = strip.Color(255, 0, 255), cyan = strip.Color(0, 255, 255), yellow = strip.Color(255, 255, 0), gold = strip.Color(255, 200, 0);
	for (int n = 0; n < NUM_LEDS; n++)
	{
		strip.setPixelColor(n, 10, 10, 10);
		strip.show();
	}

}
void loop()
{

	
}