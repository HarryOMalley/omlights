#include "FastLED.h"
#define NUM_LEDS 300
CRGB leds[NUM_LEDS];
void setup() 
{ 
	FastLED.addLeds<NEOPIXEL, 7>(leds, NUM_LEDS); 
	FastLED.setBrightness(120);
}
void loop() {
	FastLED.clear();
	for (int led = 0; led < NUM_LEDS; led++) {
		leds[led] = CRGB::Crimson;
	}
	FastLED.show();
}