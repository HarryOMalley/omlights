#include "Functions.h"
#include <Adafruit_NeoPixel.h>
#define PIN 6
#define NUM_LEDS 300
char inString[20], inChar, exitString[] = "exit";

String programString;
int colour[3];

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);


void setup() {

	Serial.begin(9600);
	Serial.println("Starting...");
	strip.begin();
	strip.setBrightness(255);
	for (int n = 0; n < NUM_LEDS; n++)
	{
		strip.setPixelColor(n, 20, 20, 20);
		strip.show();
	}
}

void loop() {

	int program = chooseProgram();
	int stop = 0;

	switch (program)
	{
	case 1:
		Serial.println("\n\n\nSingle Colour Mode\n\n");
		while (stop == 0)
		{
			getInput();
			if (checkWords(inString) == 1)
			{
				Serial.println("I should stop now...");
				stop = 1;
				break;
			}
			else if (checkWords(inString) == 2)
			{
				Serial.print("Input brightness value (0-255): ");
				while (!Serial.available()); // hang program until a byte is received notice the ; after the while()
				{}
				int brightness = Serial.parseInt();
				Serial.print("Setting brightness to: ");
				Serial.println(brightness);
				strip.setBrightness(brightness);
				strip.show();
			}
			else 
			{
				parseInt(inString);
				changeColour(colour);
			}
		}
		break;
	case 2:
		while (stop == 0)
		{
			rainbowCycle(3);
			break;
		}
		break;
	default:
		Serial.println("Error: Invalid number entered. Please reselect...");
		delay(500);
		chooseProgram();
		break;
		/*case 3:
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
			break;*/

	}

}

void changeColour(int colour[3])
{
	int R = colour[0];
	int G = colour[1];
	int B = colour[2];
	Serial.println("Setting Colour to: ");
	Serial.print("R: ");
	Serial.println(R);
	Serial.print("G: ");
	Serial.println(G);
	Serial.print("B: ");
	Serial.println(B);


	for (int n = 0; n < NUM_LEDS; n++)
	{
		strip.setPixelColor(n, R, G, B);
		strip.show();
	}
}

int chooseProgram(void)
{
	Serial.println("Available programs:");
	Serial.println("1. Simple Colour Changer");
	Serial.println("2. Flash");
	Serial.println("3. Rainbow");
	Serial.println("Please select desired program: ");
	while (Serial.available() == 0)
	{

		int program = Serial.parseInt();

		if (program > 0)
		{
			Serial.print("Launching Program ");
			Serial.print(program);
			Serial.print("...");
			delay(2000);
			return program;
		}
	}
}
int rainbowCycle(uint8_t wait) {
	uint16_t i, j;
	uint16_t randnum[strip.numPixels()];
	for (i = 0; i < strip.numPixels(); i++)
	{
		randnum[i] = random(256);
	}
	while (true)
	{
		for (j = 0; j < 256 * 100; j++)
		{ // 5 cycles of all colors on wheel - i think this just makes it do it over and over
			for (i = 0; i < strip.numPixels(); i++)
			{
				strip.setPixelColor(i, Wheel(((randnum[i] * 256 / strip.numPixels()) + j) & 255));
			}
			strip.show();
			delay(wait);
			if (Serial.available() > 0)
			{
				getInput();
				if (checkWords(inString) == 1)
				{
					return 0;
				}
			}
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

int getInput(void)
{
	byte index = 0;
	Serial.flush(); //flush all previous received and transmitted data
	resetString(inString);
	while (!Serial.available()); // hang program until a byte is received notice the ; after the while()
	{}

	for (int i = 0; i < 19; i++)
	{
		inChar = Serial.read(); // Read a character
		inString[index] = inChar; // Store it
		index++;
		inString[index] = '\0'; // Null terminate the string
		/*Serial.println(inChar);
		Serial.print("I received: ");
		Serial.println(inString);*/
		delay(10);
		if (Serial.available() == 0)
			break;
	}
	return 0;
}

void resetString(String x)
{
	memset(&x, 0, sizeof(x));
}

int checkWords(char x[])
{
	char exitString[] = "exit";
	char bString[] = "brightness";
	if (strcmp(x, exitString) == 0)
	{
		return 1;
	}
	else if (strcmp(x, bString) == 0)
	{
		return 2;
	}
	else
	{
		return 0;
	}
}
int * parseInt(char x[])
{
	int i = 1; // Counter
	char RChar[10], GChar[10], BChar[10]; // 3 Colour Char Arrays
	String numbers[3];

	String n = strtok(x, " "); // Splitting char array (x) into tokens
	numbers[0] = n; // Add it to String array
	//Serial.println(numbers[0]);
	n = strtok(NULL, " "); // Doing the same for the final 2 numbers
	numbers[1] = n;

	n = strtok(NULL, " "); // Doing the same for the final 2 numbers
	numbers[2] = n;

	// Converting from String to char
	strcpy(RChar, numbers[0].c_str());
	strcpy(GChar, numbers[1].c_str());
	strcpy(BChar, numbers[2].c_str());
	// Converting from char to int and putting in colour array
	/*Serial.println("Characters: ");
	Serial.println(RChar);
	Serial.println(GChar);
	Serial.println(BChar);*/
	colour[0] = atoi(RChar);
	colour[1] = atoi(GChar);
	colour[2] = atoi(BChar);
	return colour; // Pass the array back to main function for colour change
}



// Did use this in parseInt however it was getting stuck inside the while loop, so due to it only being 3 
// words I ditched it. 
	//while (n != NULL)
	//{
	//	Serial.print("I have received: ");
	//	n = strtok(NULL, " "); // Doing the same for the final 2 numbers
	//	numbers[i] = n;
	//	Serial.println(numbers[i]);
	//	i++;
	//}
