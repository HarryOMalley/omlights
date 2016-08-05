#include <Adafruit_NeoPixel.h>
#define PIN 6
#define NUM_LEDS 300
char R;
String end;
int n, i, G, B, program, stop = 0, colour[3];

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
	Serial.begin(9600);
	Serial.println("Starting...");
	strip.begin();
	for (n = 0; n < NUM_LEDS; n++)
	{
		strip.setPixelColor(n, 0, 0, 100);
	}
	strip.show(); // Initialize all pixels to 'off'
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
			if (Serial.available()) {
				for (i = 0; i < 3; i++)
				{
					//colour[i] = Serial.parseInt();
					//R = Serial.read();
					end = Serial.readStringUntil('\n');
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
						Serial.print("I received: ");
						Serial.println(colour[i], DEC);
					}
				}
				changeColour(colour);
			}
		}
		chooseProgram();

		break;
	default:
		Serial.println("Error: Invalid number entered. Please reselect...");
		delay(500);
		chooseProgram();
		break;

	case 2:
		Serial.println("Nothing to see here...");
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
	Serial.println("3. something else");
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