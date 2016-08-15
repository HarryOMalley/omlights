#include <Adafruit_NeoPixel.h>
#define PIN 6
#define NUM_LEDS 300
char R;
String end;
int n, i, G, B, program, stop = 0, colour[3], red, green, blue;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
	Serial.begin(9600);
	Serial.println("Starting...");
	strip.begin();
	for (n = 0; n < NUM_LEDS; n++)
	{
		strip.setPixelColor(n, 20, 20, 20);
		strip.show();
	}
	// Initialize all pixels to 'off'
	chooseProgram();
}

void loop() {

	switch (program)
	{
	case 1:
		stop = 0;
		Serial.println("\n\n********** Simple Colour Changer **********");
		while (stop == 0)
		{
			if (Serial.available())
			{
				for (i = 0; i < 3; i++)
				{

					colour[i] = Serial.parseInt();
					//R = Serial.read();
					//end = Serial.readStringUntil('\n');
					//exit.concat(R);
					if (end == "exit")
					{
						stop = 1;
						break;
					}
					else
					{

						//R = R - '0';
						// say what you got:
						//Serial.print("I received: ");
						//Serial.println(colour[i], DEC);
					}
				}
				changeColour(colour);

			}
			//Serial.println(".");n
		}
		chooseProgram();

		break;
	default:
		Serial.println("Error: Invalid number entered. Please reselect...");
		delay(500);
		chooseProgram();
		break;

	case 2:
		while (stop == 0)
		{
			rainbowCycle(3);
			if (Serial.available())
			{
				end = Serial.readStringUntil('\n');
				//exit.concat(R);
				if (end == "exit")
				{
					stop = 1;
					break;
				}
				else;
			}
		}
		break;
	case 3:
		while (stop == 0)
		{
			for (red = 0; red <= 255; red++) {
				for (green = 0; green <= 255; green++) {
					for (blue = 0; blue <= 255; blue++) {
						colour[0] = red;
						colour[1] = green;
						colour[2] = blue;
						changeColour(colour);
					}
				}
			}
		}
		break;
	}
}
void changeColour(int colour[3])
{
	R = colour[0];
	G = colour[1];
	B = colour[2];
	for (n = 0; n < NUM_LEDS; n++)
	{

		strip.setPixelColor(n, R, G, B);
		strip.show();
	}
}
/* send data only when you receive data:
if (Serial.available() > 0) {
// read the incoming byte:
incomingByte = Serial.parseInt();

// say what you got:
Serial.print("I received: ");
Serial.println(incomingByte, DEC);
} */
void chooseProgram()
{
	Serial.println("Available programs:");
	Serial.println("1. Simple Colour Changer");
	Serial.println("2. Flash");
	Serial.println("3. Rainbow");
	Serial.println("Please select desired program: ");
	while (Serial.available() == 0)
	{

		program = Serial.parseInt();

		if (program > 0)
		{
			Serial.print("Launching Program ");
			Serial.print(program);
			Serial.print("...");
			delay(2000);
			break;
		}
	}
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