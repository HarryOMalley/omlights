#include <Adafruit_NeoPixel.h>
#define PIN 7
#define NUM_LEDS 300
char inString[20], inChar, exitString[] = "exit";


String programString;
int colour[3], R, G, B;
int z = 0;
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
	//int x = 1536 / NUM_LEDS;
	//int y = 0;

	//if (y < 256)
	//{
	//	R = 255;
	//	G = y;
	//	B = 0;
	//}
	//else if (y < 512 & y > 255)
	//{
	//	R = 511 - y;
	//	G = 255;
	//	B = 0;
	//}
	//else if (y < 768);
	

	uint32_t A = Wheel(z);
	Serial.println(z);
	Serial.println(A);

	
	B = A & 255;
	Serial.println(B);
	Serial.println("\n\n");
	if (z < 255)
	{
		z++;
		delay(600);
	}
	else if (z == 255)
		z = 0;
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