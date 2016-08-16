#include <Adafruit_NeoPixel.h>
#include <stdio.h>
#include <string.h>

#define PIN 6
#define NUM_LEDS 300

char input[20], inChar, checkExit[] = "exit", RChar[20], GChar[20], BChar[20];
byte index = 0;
String programSelect;
String numbers[20], test;
int n, i, R, G, B, program, stop = 0, colour[3], red, green, blue;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
	Serial.begin(9600);
	Serial.println("Starting...");
	strip.begin();
	for (n = 0; n < NUM_LEDS; n++)
	{
		strip.setPixelColor(n, 10, 10, 10);
	}
	strip.show();
	// Initialize all pixels to 'off'
	//chooseProgram();
}
void loop() {

	Serial.flush(); //flush all previous received and transmitted data
	while (!Serial.available()); // hang program until a byte is received notice the ; after the while()
	{}

	for (i = 0; i < 19; i++)
	{
		inChar = Serial.read(); // Read a character
		input[index] = inChar; // Store it
		index++;
		input[index] = '\0'; // Null terminate the string
		Serial.println(inChar);

		Serial.print("I received: ");
		Serial.println(input);
		delay(10);
		if (Serial.available() == 0)
			break;
	}
	
	if (strcmp(input, checkExit) == 0)
		chooseProgram();
	programSelect = input;
	test = strtok(input, " ");
	numbers[0] = test;
	i = 1;
	while (test != NULL)
	{
		test = strtok(NULL, " ");
		numbers[i] = test;
		i++;
	}

	strcpy(RChar, numbers[0].c_str());
	strcpy(GChar, numbers[1].c_str());
	strcpy(BChar, numbers[2].c_str());

	R = atoi(RChar);
	G = atoi(GChar);
	B = atoi(BChar);

	colour[0] = R;
	colour[1] = G;
	colour[2] = B;

	Serial.print("Here:");
	Serial.println(R);
	Serial.println(G);
	Serial.println(B);

	changeColour(colour);


	//Serial.println(programSelect);

	//Serial.println(programSelect.toInt());
	
	memset(&input, 0, sizeof(input));
	index = 0;












	//memset(&input, 0, sizeof(input));
	//while (Serial.available() > 0) // Only read when there is data available
	//{
	//	
	//	//for (i = 0; i < 19; i++)
	//	//{
	//		inChar = Serial.read(); // Read a character
	//		input[index] = inChar; // Store it
	//		index++;
	//		input[index] = '\0'; // Null terminate the string
	//		Serial.println(inChar);
	//		
	//		Serial.print("I received: ");
	//		Serial.println(input);

	//}





	/*if (Serial.available() == 0)
			{
				index = 0;
				memset(&input, 0, sizeof(input));
			}
			Serial.print("I received: ");
			Serial.println(input);*/
			//Serial.print("I received: ");
			//Serial.println(input);



		//R = Wheel(i);   // Used to observe values when input into Wheel();
		//i++;
		////Serial.print(i);
		////Serial.print(" = ");
		//Serial.println(R);
		//delay(100);
		//if (i > 255)
		//	i = 0;






		//while (stop == 0)
			//{
			//	rainbowCycle(3);
			//	if (Serial.available())
			//	{
			//		end = Serial.readStringUntil('\n');
			//		//exit.concat(R);
			//		if (end == "exit")
			//		{
			//			stop = 1;
			//			break;
			//		}
			//		else;
			//	}
			//}

			//while (stop == 0)
			//{
			//	for (red = 0; red <= 255; red++) {
			//		for (green = 0; green <= 255; green++) {
			//			for (blue = 0; blue <= 255; blue++) {
			//				colour[0] = red;
			//				colour[1] = green;
			//				colour[2] = blue;
			//				changeColour(colour);
			//			}
			//		}
			//	}
			//}
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
	uint16_t randnum[NUM_LEDS];
	for (i = 0; i < NUM_LEDS; i++)
	{
		randnum[i] = random(256);
	}
	for (j = 0; j < 256 * 100; j++)
	{ // 5 cycles of all colors on wheel - i think this just makes it do it over and over
		for (i = 0; i < NUM_LEDS; i++)
		{
			strip.setPixelColor(i, Wheel(((randnum[i] * 256 / NUM_LEDS) + j) & 255));
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
