#include <Adafruit_NeoPixel.h>
#include <stdio.h>
#define PIN 7
#define NUM_LEDS 300
char inString[20], inChar, exitString[] = "exit";


String programString;
int colour[3], R, G, B;
int z = 0;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);


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
		
	}
strip.show();
}
void loop()
{

	int i, j;
	j = 0;
	//if (j == 0)
	//{
		for (i = 0; i < NUM_LEDS; i += 2);
		{
			strip.setPixelColor(i, 255, 0, 0, 0);
			strip.show();
		}
		for (i = 1; i < NUM_LEDS; i + 2);
		{
			strip.setPixelColor(i, 0, 255, 0, 0);
			strip.show();
		}
		delay(1000);
		j = 1;
	//}
	if (j == 1)
	{
		for (int i = 0; i = NUM_LEDS; i + 2);
		{
			strip.setPixelColor(i, 0, 255, 0, 0);
			strip.show();
		}
		for (int i = 1; i = NUM_LEDS; i + 2);
		{
			strip.setPixelColor(i, 255, 0, 0, 0);
			strip.show();
		}
		delay(1000);
		j = 0;
	}
}
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
int rainbowCycle(uint8_t wait) {
	uint16_t i, j;
	uint16_t randnum[NUM_LEDS];
	for (i = 0; i < NUM_LEDS; i++)
	{
		randnum[i] = random(256);
	}
	while (true)
	{
		for (j = 0; j < 256 * 5; j++)
		{ // 5 cycles of all colors on wheel - i think this just makes it do it over and over
			for (i = 0; i < NUM_LEDS; i++)
			{
				strip.setPixelColor(i, Wheel(((randnum[i] * 256 / NUM_LEDS) + j) & 255));
			}
			strip.show();
			delay(wait);
			if (Serial.available() > 0)
			{
				/*getInput();
				if (checkWords(inString) == 1)
				{
					return 0;
				}*/
			}
		}
	}
}

uint32_t colourCalc(uint16_t wheelPos)
{

	if (wheelPos < 256)
	{
		return strip.Color(255, wheelPos, 0);

	}
	else if (wheelPos < 512 & wheelPos > 255)
	{
		return strip.Color(511 - wheelPos, 255, 0);

	}
	else if (wheelPos < 768 & wheelPos > 511)
	{
		return strip.Color(0, 255, wheelPos - 511);

	}
	else if (wheelPos < 1024 & wheelPos > 767)
	{
		return strip.Color(0, 1023 - wheelPos, 255);

	}
	else if (wheelPos < 1280 & wheelPos > 1023)
	{
		return strip.Color(wheelPos - 1023, 0, 255);

	}
	else if (wheelPos < 1536 & wheelPos > 1279)
	{
		return strip.Color(255, 0, 1537 - wheelPos);

	}
	else
		return 1;
}