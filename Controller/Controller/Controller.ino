#include "FastLED.h"
#include <Bounce2.h>
#include <EEPROM.h>

#define NUM_LEDS 105
CRGB leds[NUM_LEDS];

Bounce debouncer1 = Bounce(); // instantiate bounce object
Bounce debouncer2 = Bounce(); // instantiate bounce object
Bounce debouncer3 = Bounce(); // instantiate bounce object

// constants won't change. They're used here to set pin numbers:
const int buttonPin1 = 2;     // the number of the pushbutton pin
const int buttonPin2 = 3;     // the number of the pushbutton pin
const int buttonPin3 = 4;     // the number of the pushbutton pin
const int ledPin = 13;      // the number of the LED pin
const int potPin = 5;
int potVal = 0;

// variables will change:
int buttonState1 = 0;         // variable for reading the pushbutton status
int buttonState2 = 0;         // variable for reading the pushbutton status
int buttonState3 = 0;         // variable for reading the pushbutton status

// move these to eeprom
bool ledOn = true;
int ledBrightness = 100; 
int ledHue = 0;
int mode = 1;
int stateAddr = 0;
int brightAddr = 1;
int modeAddr = 2;
DEFINE_GRADIENT_PALETTE(heatmap_gp) {
	0, 255, 160, 100,   //warm white
	128, 200, 200, 200,   //white
	255, 100, 160, 255,   //cool white
};
CRGBPalette16 whitePalette = heatmap_gp;


void setup()
{
	ledOn = EEPROM.read(stateAddr);
	ledBrightness = EEPROM.read(brightAddr);
	mode = EEPROM.read(modeAddr);

	FastLED.addLeds<NEOPIXEL, 7>(leds, NUM_LEDS);
	FastLED.setBrightness(180);
	FastLED.clear();
	for (int led = 0; led < NUM_LEDS; led++) {
		leds[led] = CRGB::Purple;
	}
	for (int i = 0; i < 35; i++) {

		leds[i] %= 60;
	}
	for (int i = 70; i < 105; i++) {
		leds[i] %= 60;
	}

	// initialize the LED pin as an output:
	// pinMode(ledPin, OUTPUT);
	// initialize the pushbutton pin as an input:
	pinMode(buttonPin1, INPUT);
	// initialize the pushbutton pin as an input:
	pinMode(buttonPin2, INPUT);
	// initialize the pushbutton pin as an input:
	pinMode(buttonPin3, INPUT);

	debouncer1.attach(buttonPin1, INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
	debouncer2.attach(buttonPin2, INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
	debouncer3.attach(buttonPin3, INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode

	debouncer1.interval(25); // Use a debounce interval of 25 milliseconds
	debouncer2.interval(25); // Use a debounce interval of 25 milliseconds
	debouncer3.interval(25); // Use a debounce interval of 25 milliseconds
	Serial.begin(9600);

}
void loop()
{
	// update button debouncers
	debouncer1.update();
	debouncer2.update();
	debouncer3.update();

	//Serial.println(buttonState1);
	// check if the pushbutton is pressed. If it is, the buttonState is HIGH:
	if (debouncer1.rose())
	{
		if (ledOn == true)
		{
			FastLED.setBrightness(0);
			ledOn = false;
			FastLED.show();
		}
		else
		{
			FastLED.setBrightness(ledBrightness);
			ledOn = true;
			FastLED.show();
		}
		delay(250);
	}
	else
	{
	}

	if (debouncer2.rose()) // Switch between modes
	{
		mode++;
		if (mode > 3)
		{
			mode = 1;
		}
		EEPROM.write(modeAddr, mode);
		EEPROM.write(brightAddr, ledBrightness);
		EEPROM.write(stateAddr, ledOn);
	}
	else
	{

	}

	if (debouncer3.rose()) // choose options within modes
	{
		
	}
	else
	{
		// turn LED off:
		//digitalWrite(ledPin, LOW);
	}

	run(mode);


	FastLED.show();
}

void run(int mode)
{
	potVal = analogRead(potPin);
	//Serial.println(potVal);
	int adjPot = map(potVal, 0, 1023, 0, 255);
	//Serial.println(adjPot);
	//Serial.println(ColorFromPalette(whitePalette, adjPot));

	switch (mode)
	{
	case 1: // colour
		ledHue = adjPot;
		for (int led = 0; led < NUM_LEDS; led++) {
			leds[led] = CHSV(ledHue, 255, ledBrightness);
		}
		break;
	case 2: // white

		for (int led = 0; led < NUM_LEDS; led++) {
			leds[led] = ColorFromPalette(whitePalette, adjPot); // normal palette access
			FastLED.setBrightness(ledBrightness);
		}
		break;
	case 3: // brightness
		ledBrightness = adjPot;
		for (int led = 0; led < NUM_LEDS; led++) {
			leds[led] = CHSV(ledHue, 255, ledBrightness);
		}
		break;
	default:
		break;
	}


	FastLED.show();

}