#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN 6
#define NUM_LEDS 300

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {

	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
}

void loop() {
	rainbowCycle(3);
}
void rainbowCycle(uint8_t wait) {
	uint16_t i, j;
	uint16_t randnum[strip.numPixels()];
	for (i = 0; i < strip.numPixels(); i++)
	{
		randnum[i] = random(256);
	}

	for (j = 0; j < 256 * 100; j++)
	{ // 5 cycles of all colors on wheel - i think this just makes it do it over and over
		for (i = 0; i < strip.numPixels(); i++)
		{
			strip.setPixelColor(i, Wheel(((randnum[i] * 256 / strip.numPixels()) + j) & 255));
		}
		strip.show();
		delay(wait);
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