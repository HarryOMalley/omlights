#include <Arduino.h>
#include <Adafruit_Neopixel.h>
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    7

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 100

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
uint32_t Wheel(byte WheelPos);
int rainbowCycle(uint8_t wait);
int redGreen();
void rainbow(uint8_t wait);

void setup() {
  Serial.begin(9600);
  strip.begin();
  //strip.show(); // Initialize all pixels to 'off'
  // for(int i = 0; i < LED_COUNT; i++)
  // {
  //   strip.setPixelColor(i, 80, 80, 80);
  //   strip.show();
  // }
  // put your setup code here, to run once:
}

void loop() {
  rainbow(250);
  //redGreen();
  // put your main code here, to run repeatedly:
}

int redGreen()
{
  int i, j;
  j = 0;
  while (true)
  {

    if (j == 0)
    {
      for (i = 0; i < LED_COUNT; i = i + 2)
      {
        strip.setPixelColor(i, 255, 0, 0);
      }
      for (int i = 1; i < LED_COUNT; i = i + 2)
      {
        strip.setPixelColor(i, 0, 255, 0);
      }
      strip.show();
      delay(1000);
      j = 1;
    }
    else if (j == 1)
    {
      for (int i = 0; i < LED_COUNT; i = i + 2)
      {
        strip.setPixelColor(i, 0, 255, 0);
      }
      for (int i = 1; i < LED_COUNT; i = i + 2)
      {
        strip.setPixelColor(i, 255, 0, 0);
      }
      strip.show();
      delay(1000);
      j = 0;
    }
  }
}

int rainbowCycle(uint8_t wait) 
{
	uint16_t i, j;
	uint16_t randnum[LED_COUNT];
	for (i = 0; i < LED_COUNT; i++)
	{
		randnum[i] = random(256);
	}
	while (true)
	{
		for (j = 0; j < 256 * 100; j++)
		{ // 5 cycles of all colors on wheel - i think this just makes it do it over and over
			for (i = 0; i < LED_COUNT; i++)
			{
				strip.setPixelColor(i, Wheel(((randnum[i] * 256 / LED_COUNT) + j) & 255));
			}
			strip.show();
			delay(wait);
			// if (Serial.available() > 0)
			// {
			// 	getInput();
			// 	if (checkWords(inString) == 1)
			// 	{
			// 		return 0;
			// 	}
			// }
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

uint32_t Wheel2(byte WheelPos)
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return strip.Color(255 - WheelPos * 3, WheelPos * 3, 0);
	}
	if (WheelPos < 170) {
		WheelPos = 170 - 85;
		return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
	WheelPos -= 170;
	return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}



void rainbow(uint8_t wait) {
	uint16_t i, j;

	for (j = 0; j<255; j++) {
		for (i = 0; i<strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel((i + j) & 255));
		}
		strip.show();
		delay(wait);
	}
  return;
}
