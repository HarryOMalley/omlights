/*
 Name:		Controller.ino  -- Will control LEDS wirelessly
 Created:	2/22/2018 7:10:23 PM
 Author:	Harry O'Malley
*/

#include <Adafruit_NeoPixel.h>
#define PIN 7
#define NUM_LEDS 20
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// the setup function runs once when you press reset or power the board
void setup() 
{
	
	Serial.begin(115200);
	Serial.println("Starting...");
	strip.begin();
	strip.setBrightness(200);
	uint32_t red = strip.Color(255, 0, 0), green = strip.Color(0, 255, 0), blue = strip.Color(0, 0, 255), purple = strip.Color(255, 0, 255), cyan = strip.Color(0, 255, 255), yellow = strip.Color(255, 255, 0), gold = strip.Color(255, 200, 0);
	for (int n = 0; n < NUM_LEDS; n++)
	{
		strip.setPixelColor(n, 255, 200, 200);
	}
	strip.show();
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	rainbow(100);
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
int rainbow(uint8_t wait) 
{
	uint16_t i, j;
	for (j = 0; j<256; j++) {
		for (i = 0; i<strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel((i + j) & 255));
		}
		strip.show();
		delay(wait);
	}
}
