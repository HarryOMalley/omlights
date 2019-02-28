#include "FastLED.h"
#define NUM_LEDS 105
CRGB leds[NUM_LEDS];

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

bool ledOn = true;
int ledBrightness = 100;
int ledHue = 0;
void setup()
{
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


}
void loop()
{
	// read the state of the pushbutton value:
	buttonState1 = digitalRead(buttonPin1);
	// read the state of the pushbutton value:
	buttonState2 = digitalRead(buttonPin2);
	// read the state of the pushbutton value:
	buttonState3 = digitalRead(buttonPin3);

	potVal = analogRead(potPin);

	Serial.println(potVal);
	ledHue = map(potVal, 0, 1023, 0, 255);

	for (int led = 0; led < NUM_LEDS; led++) {
		leds[led] = CHSV(ledHue, 255, 255);
	}

	//Serial.println(buttonState1);
	// check if the pushbutton is pressed. If it is, the buttonState is HIGH:
	if (buttonState1 == HIGH)
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
		// turn LED off:
		//digitalWrite(ledPin, LOW);
	}

	if (buttonState2 == HIGH)
	{
		// turn LED on:
		//digitalWrite(ledPin, HIGH);
		Serial.println("2 ON");
	}
	else
	{
		// turn LED off:
		//digitalWrite(ledPin, LOW);
	}

	if (buttonState3 == HIGH)
	{
		// turn LED on:
		//digitalWrite(ledPin, HIGH);
		Serial.println("3 ON");
	}
	else
	{
		// turn LED off:
		//digitalWrite(ledPin, LOW);
	}




	FastLED.show();
}