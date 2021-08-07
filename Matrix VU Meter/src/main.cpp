// (Heavily) adapted from https://github.com/G6EJD/ESP32-8266-Audio-Spectrum-Display/blob/master/ESP32_Spectrum_Display_02.ino
// Adjusted to allow brightness changes on press+hold, Auto-cycle for 3 button presses within 2 seconds
// Edited to add Neomatrix support for easier compatibility with different layouts.
#define FASTLED_INTERNAL
#include <Arduino.h>
#include <Wire.h>
#include <FastLED_NeoMatrix.h>
#include <arduinoFFT.h>
#include <EasyButton.h>
#include "WiFi.h"
#include <PubSubClient.h>
const char* ssid = "Lima";
const char* password =  "Limesaregreat";

const char* mqtt_server = "192.168.1.167";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


#define SAMPLES         512          // Must be a power of 2
#define SAMPLING_FREQ   36000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AMPLITUDE       2000        // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
#define AUDIO_IN_PIN    35            // Signal in on this pin
#define LED_PIN         5             // LED strip data
#define BTN_PIN         4             // Connect a push button to this pin to change patterns
#define LONG_PRESS_MS   200           // Number of ms to count as a long press
#define COLOR_ORDER     GRB           // If colours look wrong, play with this
#define CHIPSET         WS2812B       // LED strip type
#define MAX_MILLIAMPS   35000          // Careful with the amount of power here if running off USB port
const int BRIGHTNESS_SETTINGS[3] = {5, 100, 255};  // 3 Integer array for 3 brightness settings (based on pressing+holding BTN_PIN)
#define LED_VOLTS       5             // Usually 5 or 12
#define NUM_BANDS       33            // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
#define NOISE           200           // Used as a crude noise filter, values below this are ignore

#define MATRIX_TILE_WIDTH   22 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT  22 // height of each matrix
#define MATRIX_TILE_H       3  // number of matrices arranged horizontally
#define MATRIX_TILE_V       1  // number of matrices arranged vertically

// Used by NeoMatrix
#define MATRIX_WIDTH (MATRIX_TILE_WIDTH *  MATRIX_TILE_H)
#define MATRIX_HEIGHT (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define NUM_MATRIX (MATRIX_WIDTH*MATRIX_HEIGHT)

// Compat for some other demos
#define NUM_LEDS NUM_MATRIX

#define BAR_WIDTH      (MATRIX_WIDTH  / (NUM_BANDS - 1))  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define TOP            (MATRIX_HEIGHT - 0)                // Don't allow the bars to go offscreen
#define SERPENTINE     true                               // Set to false if you're LEDS are connected end to end, true if serpentine



#define LED_BLACK		0

#define LED_RED_VERYLOW 	(3 <<  11)
#define LED_RED_LOW 		(7 <<  11)
#define LED_RED_MEDIUM 		(15 << 11)
#define LED_RED_HIGH 		(31 << 11)

#define LED_GREEN_VERYLOW	(1 <<  5)
#define LED_GREEN_LOW 		(15 << 5)
#define LED_GREEN_MEDIUM 	(31 << 5)
#define LED_GREEN_HIGH 		(63 << 5)

#define LED_BLUE_VERYLOW	3
#define LED_BLUE_LOW 		7
#define LED_BLUE_MEDIUM 	15
#define LED_BLUE_HIGH 		31

#define LED_ORANGE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW		(LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW	(LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW		(LED_RED_LOW     + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM	(LED_RED_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH		(LED_RED_HIGH    + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW	(LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW		(LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM		(LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH		(LED_GREEN_HIGH    + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW		(LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)


// Sampling and FFT stuff
// unsigned int sampling_period_us;
// byte peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};              // The length of these arrays must be >= NUM_BANDS
// int oldBarHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// int bandValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// double vReal[SAMPLES];
// double vImag[SAMPLES];
// unsigned long newTime;
// arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);

unsigned int sampling_period_us;
byte peak[MATRIX_WIDTH];              // The length of these arrays must be >= NUM_BANDS
int oldBarHeights[MATRIX_WIDTH];
int bandValues[MATRIX_WIDTH];
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime;
arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);

// Button stuff
int buttonPushCounter = 0;
bool autoChangePatterns = false;
EasyButton modeBtn(BTN_PIN);

uint8_t mode = 0;
uint8_t strobeStatus = 0;
uint16_t strobeRate = 200;

// FastLED stuff
CRGB leds[NUM_LEDS];
DEFINE_GRADIENT_PALETTE( purple_gp ) {
  0,   0, 212, 255,   //blue
255, 179,   0, 255 }; //purple
DEFINE_GRADIENT_PALETTE( outrun_gp ) {
  0, 141,   0, 100,   //purple
127, 255, 192,   0,   //yellow
255,   0,   5, 255 };  //blue
DEFINE_GRADIENT_PALETTE( greenblue_gp ) {
  0,   0, 255,  60,   //green
 64,   0, 236, 255,   //cyan
128,   0,   5, 255,   //blue
192,   0, 236, 255,   //cyan
255,   0, 255,  60 }; //green
DEFINE_GRADIENT_PALETTE( redyellow_gp ) {
  0,   200, 200,  200,   //white
 64,   255, 218,    0,   //yellow
128,   231,   0,    0,   //red
192,   255, 218,    0,   //yellow
255,   200, 200,  200 }; //white
CRGBPalette16 purplePal = purple_gp;
CRGBPalette16 outrunPal = outrun_gp;
CRGBPalette16 greenbluePal = greenblue_gp;
CRGBPalette16 heatPal = redyellow_gp;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// PALETTES

// Gradient palette "YlOrBr_03_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cb/seq/tn/YlOrBr_03.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( YlOrBr_03_gp ) {
    0, 255,237,119,
   84, 255,237,119,
   84, 252,142, 13,
  170, 252,142, 13,
  170, 167, 29,  1,
  255, 167, 29,  1};

// Gradient palette "RdPu_03_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cb/seq/tn/RdPu_03.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( RdPu_03_gp ) {
    0, 249,191,178,
   84, 249,191,178,
   84, 242, 90,108,
  170, 242, 90,108,
  170, 130,  1, 54,
  255, 130,  1, 54};

// Gradient palette "BuGn_03_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cb/seq/tn/BuGn_03.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( BuGn_03_gp ) {
    0, 192,233,240,
   84, 192,233,240,
   84,  67,176,140,
  170,  67,176,140,
  170,   2, 93, 21,
  255,   2, 93, 21};

// Gradient palette "GMT_panoply_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gmt/tn/GMT_panoply.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 128 bytes of program space.

DEFINE_GRADIENT_PALETTE( GMT_panoply_gp ) {
    0,   1,  1,168,
   15,   1,  1,168,
   15,   1, 19,255,
   31,   1, 19,255,
   31,   7, 79,255,
   47,   7, 79,255,
   47,  27,138,255,
   63,  27,138,255,
   63,  47,178,255,
   79,  47,178,255,
   79,  71,219,255,
   95,  71,219,255,
   95,  95,233,255,
  111,  95,233,255,
  111, 146,255,255,
  127, 146,255,255,
  127, 255,252, 10,
  143, 255,252, 10,
  143, 255,213,  0,
  159, 255,213,  0,
  159, 255,142,  0,
  175, 255,142,  0,
  175, 255, 72,  0,
  191, 255, 72,  0,
  191, 255, 15,  0,
  207, 255, 15,  0,
  207, 255,  0,  0,
  223, 255,  0,  0,
  223, 159,  0,  0,
  239, 159,  0,  0,
  239,  73,  0,  0,
  255,  73,  0,  0};

// Gradient palette "pm3d07_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pm/tn/pm3d07.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( pm3d07_gp ) {
    0,   0,255,  0,
   84,   0, 32,  0,
   84, 255,255,  0,
  170, 135,149,  0,
  170, 255,  0,  0,
  255,  52,  0,  0};

// Gradient palette "Life_is_beautiful_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/katiekat013/tn/Life_is_beautiful.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( Life_is_beautiful_gp ) {
    0,   2, 37, 16,
   51,   2, 37, 16,
   51,   3, 62, 11,
  102,   3, 62, 11,
  102,  42, 68,  6,
  153,  42, 68,  6,
  153, 109,112,  2,
  204, 109,112,  2,
  204, 242,100,  0,
  255, 242,100,  0};

// Gradient palette "min_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/esdb/tn/min.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( min_gp ) {
    0, 255,255,  0,
   84, 255,255,  0,
   84, 101,255, 45,
  170, 101,255, 45,
  170,  42,255,255,
  255,  42,255,255};

// Gradient palette "seismic_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gery/tn/seismic.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 216 bytes of program space.

DEFINE_GRADIENT_PALETTE( seismic_gp ) {
    0, 255,  4,  2,
   40, 255,  4,  2,
   40, 255, 34, 26,
   61, 255, 34, 26,
   61, 255, 66, 54,
   75, 255, 66, 54,
   75, 255, 92, 80,
   85, 255, 92, 80,
   85, 255,115,103,
   92, 255,115,103,
   92, 255,135,123,
   98, 255,135,123,
   98, 255,154,144,
  103, 255,154,144,
  103, 255,169,160,
  108, 255,169,160,
  108, 255,186,178,
  112, 255,186,178,
  112, 255,201,194,
  116, 255,201,194,
  116, 255,217,212,
  120, 255,217,212,
  120, 255,231,228,
  123, 255,231,228,
  123, 255,246,245,
  127, 255,246,245,
  127, 255,255,255,
  127, 255,255,255,
  127, 247,248,255,
  130, 247,248,255,
  130, 229,233,255,
  134, 229,233,255,
  134, 210,217,255,
  138, 210,217,255,
  138, 194,203,255,
  142, 194,203,255,
  142, 177,187,255,
  146, 177,187,255,
  146, 159,171,255,
  150, 159,171,255,
  150, 140,154,255,
  156, 140,154,255,
  156, 121,136,255,
  162, 121,136,255,
  162, 100,115,255,
  169, 100,115,255,
  169,  77, 92,255,
  179,  77, 92,255,
  179,  50, 65,255,
  193,  50, 65,255,
  193,  23, 33,255,
  214,  23, 33,255,
  214,   2,  4,255,
  255,   2,  4,255};

// Gradient palette "Romanian_flag_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ggr/tn/Romanian_flag.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

DEFINE_GRADIENT_PALETTE( Romanian_flag_gp ) {
    0,   0,  0,255,
   42,   0,  0,255,
   84,   0,  0,255,
   84, 255,255,  0,
  127, 255,255,  0,
  170, 255,255,  0,
  170, 255,  0,  0,
  212, 255,  0,  0,
  255, 255,  0,  0};

// Gradient palette "cmy_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/imagej/tn/cmy.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 1052 bytes of program space.

DEFINE_GRADIENT_PALETTE( cmy_gp ) {
    0,   0,  0,  0,
    0,   0,  0,  0,
    0,   0,255,255,
    1,   0,255,255,
    2,   0,255,255,
    3,   0,255,255,
    4,   0,255,255,
    5,   0,255,255,
    6,   0,255,255,
    7,   0,255,255,
    8,   0,255,255,
    9,   0,255,255,
   10,   0,255,255,
   11,   0,255,255,
   12,   0,255,255,
   13,   0,255,255,
   14,   0,255,255,
   15,   0,255,255,
   16,   0,255,255,
   17,   0,255,255,
   18,   0,255,255,
   19,   0,255,255,
   20,   0,255,255,
   21,   0,255,255,
   22,   0,255,255,
   23,   0,255,255,
   24,   0,255,255,
   25,   0,255,255,
   26,   0,255,255,
   27,   0,255,255,
   28,   0,255,255,
   29,   0,255,255,
   30,   0,255,255,
   31,   0,255,255,
   32,   0,255,255,
   33,   0,255,255,
   34,   0,255,255,
   35, 0, 255,255,
  210, 255,255,  0,
  211, 255,255,  0,
  212, 255,255,  0,
  213, 255,255,  0,
  214, 255,255,  0,
  215, 255,255,  0,
  216, 255,255,  0,
  217, 255,255,  0,
  218, 255,255,  0,
  219, 255,255,  0,
  220, 255,255,  0,
  221, 255,255,  0,
  222, 255,255,  0,
  223, 255,255,  0,
  224, 255,255,  0,
  225, 255,255,  0,
  226, 255,255,  0,
  227, 255,255,  0,
  228, 255,255,  0,
  229, 255,255,  0,
  230, 255,255,  0,
  231, 255,255,  0,
  232, 255,255,  0,
  233, 255,255,  0,
  234, 255,255,  0,
  235, 255,255,  0,
  236, 255,255,  0,
  237, 255,255,  0,
  238, 255,255,  0,
  239, 255,255,  0,
  240, 255,255,  0,
  241, 255,255,  0,
  242, 255,255,  0,
  243, 255,255,  0,
  244, 255,255,  0,
  245, 255,255,  0,
  246, 255,255,  0,
  247, 255,255,  0,
  248, 255,255,  0,
  249, 255,255,  0,
  250, 255,255,  0,
  251, 255,255,  0,
  252, 255,255,  0,
  252, 217,223,219,
  253, 217,223,219,
  253, 237,239,237,
  254, 237,239,237,
  254, 255,255,255,
  255, 255,255,255};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// PALETTES END

CRGBPalette16 palette1 = YlOrBr_03_gp;
CRGBPalette16 palette2 = RdPu_03_gp;
CRGBPalette16 palette3 = BuGn_03_gp;
CRGBPalette16 palette4 = GMT_panoply_gp;
CRGBPalette16 palette5 = pm3d07_gp;
CRGBPalette16 palette6 = Life_is_beautiful_gp;
CRGBPalette16 palette7 = min_gp;
CRGBPalette16 palette8 = seismic_gp;
CRGBPalette16 palette9 = Romanian_flag_gp;
CRGBPalette16 palette10 = cmy_gp;


CRGBPalette16 paletteList[] = {palette1, palette2, palette3, palette4, palette5, palette6, palette7, palette8, palette9, palette10};

CRGBPalette16 currentPalette = paletteList[0];

uint8_t colorTimer = 0;

// FastLED_NeoMaxtrix - see https://github.com/marcmerlin/FastLED_NeoMatrix for Tiled Matrixes, Zig-Zag and so forth
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, MATRIX_TILE_H, MATRIX_TILE_V, 
  NEO_MATRIX_TOP        + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS       + NEO_MATRIX_ZIGZAG +
  NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS + NEO_TILE_PROGRESSIVE);

void reconnect();
void initialiseArrays();
void changeMode();
void startAutoMode();
void brightnessButton();
void brightnessOff();
void rainbowBars(int band, int barHeight);
void purpleBars(int band, int barHeight);
void changingBars(int band, int barHeight);
void centerBars(int band, int barHeight);
void whitePeak(int band);
void outrunPeak(int band);
void waterfall(int band);
void mqttCallback(char* topic, byte* message, unsigned int length);
void paintWhite();
void paintBlack();

void setup() {
  initialiseArrays();
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, MAX_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS_SETTINGS[0]);
  FastLED.clear();

  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
  // client.setCredentials( "user", "test"); 
  // modeBtn.begin();
  // modeBtn.onPressed(changeMode);
  // modeBtn.onPressedFor(LONG_PRESS_MS, brightnessButton);
  // modeBtn.onSequence(3, 2000, startAutoMode);
  // modeBtn.onSequence(5, 2000, brightnessOff);
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQ));
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", "user", "test")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("matrix/brightness");
      client.subscribe("strobe");
      client.subscribe("strobe/rate");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "matrix/brightness") {
    // Serial.print("Changing output to ");
    if(messageTemp == "2"){
      Serial.println("on");
      FastLED.setBrightness(BRIGHTNESS_SETTINGS[2]);
    }
    else if(messageTemp == "1"){
      Serial.println("on");
      FastLED.setBrightness(BRIGHTNESS_SETTINGS[1]);
    }
    else if(messageTemp == "0"){
      Serial.println("on");
      FastLED.setBrightness(BRIGHTNESS_SETTINGS[0]);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      FastLED.setBrightness(0);
    }
  }

  if (String(topic) == "strobe") {
    if(messageTemp == "on") {
      mode = 1;
    } else {
      mode = 0;
    }
  }
if (String(topic) == "strobe/rate") {
    strobeRate = messageTemp.toInt();
  }
}

void initialiseArrays() {
  for(int i=0; i<MATRIX_WIDTH; i++){
      peak[i] = 0;
      oldBarHeights[i] = 0;
      bandValues[i] = 0;
  }
}

void changeMode() {
  // Serial.println("Button pressed");
  if (FastLED.getBrightness() == 0) FastLED.setBrightness(BRIGHTNESS_SETTINGS[0]);  //Re-enable if lights are "off"
  autoChangePatterns = false;



  buttonPushCounter = (buttonPushCounter + 1) % 6;
}

void startAutoMode() {
  autoChangePatterns = true;
}

void brightnessButton() {
  if (FastLED.getBrightness() == BRIGHTNESS_SETTINGS[2])  FastLED.setBrightness(BRIGHTNESS_SETTINGS[0]);
  else if (FastLED.getBrightness() == BRIGHTNESS_SETTINGS[0]) FastLED.setBrightness(BRIGHTNESS_SETTINGS[1]);
  else if (FastLED.getBrightness() == BRIGHTNESS_SETTINGS[1]) FastLED.setBrightness(BRIGHTNESS_SETTINGS[2]);
  else if (FastLED.getBrightness() == 0) FastLED.setBrightness(BRIGHTNESS_SETTINGS[0]); //Re-enable if lights are "off"
}

void brightnessOff(){
  FastLED.setBrightness(0);  //Lights out
}

/********************************
                   MAIN LOOP
**************************************/
void loop() {

  if (!client.connected()) {
      reconnect();
    }
  client.loop();

  if(mode == 1) { //STROBE
     delay(strobeRate);
    if(strobeStatus == 0) {
      paintWhite();
      FastLED.show();
      strobeStatus = 1;
    } else {
      paintBlack();
      FastLED.show();
      strobeStatus = 0;
    }
  } else {


  // Don't clear screen if waterfall pattern, be sure to change this is you change the patterns / order
  // if (buttonPushCounter != 5) FastLED.clear();

  // modeBtn.read();

  // Reset bandValues[]
  for (int i = 0; i<NUM_BANDS; i++){
    bandValues[i] = 0;
  }

  // Sample the audio pin
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros();
    vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 9.7uS on an ESP32
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* chill */ }
  }

  // Compute FFT
  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();

  // Analyse FFT results
  for (int i = 2; i < (SAMPLES/2); i++){       // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
    if (vReal[i] > NOISE) {                    // Add a crude noise filter

    if(i <= 2) bandValues[0] += (int)vReal[i];
    if(i>=2 && i <= 3) bandValues[1] += (int)vReal[i];
    if(i>=3 && i <= 4) bandValues[2] += (int)vReal[i];
    if(i>=4 && i <= 5) bandValues[3] += (int)vReal[i];
    if(i>=5 && i <= 6) bandValues[4] += (int)vReal[i];
    if(i>=6 && i <= 7) bandValues[5] += (int)vReal[i];
    if(i>=7 && i <= 8) bandValues[6] += (int)vReal[i];
    if(i>=8 && i <= 9) bandValues[7] += (int)vReal[i];
    if(i>=9 && i <= 10) bandValues[8] += (int)vReal[i];
    if(i>=10 && i <= 11) bandValues[9] += (int)vReal[i];
    if(i>=11 && i <= 12) bandValues[10] += (int)vReal[i];
    if(i>=12 && i <= 13) bandValues[11] += (int)vReal[i];
    if(i>=13 && i <= 14) bandValues[12] += (int)vReal[i];
    if(i>=14 && i <= 15) bandValues[13] += (int)vReal[i];
    if(i>=15 && i <= 16) bandValues[14] += (int)vReal[i];
    if(i>=16 && i <= 17) bandValues[15] += (int)vReal[i];
    if(i>=17 && i <= 18) bandValues[16] += (int)vReal[i];
    if(i>=18 && i <= 20) bandValues[17] += (int)vReal[i];
    if(i>=20 && i <= 24) bandValues[18] += (int)vReal[i];
    if(i>=24 && i <= 28) bandValues[19] += (int)vReal[i];
    if(i>=28 && i <= 34) bandValues[20] += (int)vReal[i];
    if(i>=34 && i <= 40) bandValues[21] += (int)vReal[i];
    if(i>=40 && i <= 47) bandValues[22] += (int)vReal[i];
    if(i>=47 && i <= 55) bandValues[23] += (int)vReal[i];
    if(i>=55 && i <= 65) bandValues[24] += (int)vReal[i];
    if(i>=65 && i <= 77) bandValues[25] += (int)vReal[i];
    if(i>=77 && i <= 91) bandValues[26] += (int)vReal[i];
    if(i>=91 && i <= 108) bandValues[27] += (int)vReal[i];
    if(i>=108 && i <= 127) bandValues[28] += (int)vReal[i];
    if(i>=127 && i <= 150) bandValues[29] += (int)vReal[i];
    if(i>=150 && i <= 178) bandValues[30] += (int)vReal[i];
    if(i>=178 && i <= 210) bandValues[31] += (int)vReal[i];
    if(i>=210 && i <= 248) bandValues[32] += (int)vReal[i];


    }
  }

  // Process the FFT data into bar heights
  for (byte band = 0; band < NUM_BANDS; band++) {

    // Scale the bars for the display
    int barHeight = bandValues[band] / AMPLITUDE;
    if (barHeight > TOP) barHeight = TOP;

    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min(TOP, barHeight);
    }

    // Draw bars
    switch (buttonPushCounter) {
      case 0:
        rainbowBars(band, barHeight);
        break;
      case 1:
        // No bars on this one
        break;
      case 2:
        purpleBars(band, barHeight);
        break;
      case 3:
        centerBars(band, barHeight);
        break;
      case 4:
        changingBars(band, barHeight);
        break;
      case 5:
        waterfall(band);
        break;
    }

    // Draw peaks
    switch (buttonPushCounter) {
      case 0:
        whitePeak(band);
        break;
      case 1:
        outrunPeak(band);
        break;
      case 2:
        whitePeak(band);
        break;
      case 3:
        // No peaks
        break;
      case 4:
        // No peaks
        break;
      case 5:
        // No peaks
        break;
    }

    // Save oldBarHeights for averaging later
    oldBarHeights[band] = barHeight;
      }
  }

  // Decay peak
  EVERY_N_MILLISECONDS(60) {
    for (byte band = 0; band < NUM_BANDS; band++)
      if (peak[band] > 0) peak[band] -= 1;
    colorTimer++;
  }

  // Used in some of the patterns
  EVERY_N_MILLISECONDS(10) {
    colorTimer++;
  }

  EVERY_N_SECONDS(10) {
    if (autoChangePatterns) buttonPushCounter = (buttonPushCounter + 1) % 6;
  }

  FastLED.show();
}

// PATTERNS BELOW //

void paintWhite() {
  // matrix->fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, LED_WHITE_HIGH);
  for(int i=0; i<MATRIX_WIDTH; i++){
      for(int j=0; j<MATRIX_HEIGHT; j++){
          matrix->drawPixel(i, j, CRGB(255, 255, 255));
      }
  }
}

void paintBlack() {
  // matrix->fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, LED_BLACK);
    for(int i=0; i<MATRIX_WIDTH; i++){
      for(int j=0; j<MATRIX_HEIGHT; j++){
          matrix->drawPixel(i, j, CRGB(0, 0, 0));
      }
  }
}

void rainbowBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix->drawPixel(x, y, CHSV((x / BAR_WIDTH) * (255 / NUM_BANDS), 255, 255));
    }
  }
}

void purpleBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix->drawPixel(x, y, ColorFromPalette(purplePal, y * (255 / (barHeight + 1))));
    }
  }
}

void paletteBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix->drawPixel(x, y, ColorFromPalette(currentPalette, y * (255 / (barHeight + 1))));
    }
  }
}

void changingBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix->drawPixel(x, y, CHSV(y * (255 / MATRIX_TILE_HEIGHT) + colorTimer, 255, 255));
    }
  }
}

void centerBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    int yStart = ((MATRIX_TILE_HEIGHT - barHeight) / 2 );
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix->drawPixel(x, y, ColorFromPalette(heatPal, colorIndex));
    }
  }
}

void whitePeak(int band) {
  int xStart = BAR_WIDTH * band;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    matrix->drawPixel(x, peakHeight, CHSV(0,0,255));
  }
}

void outrunPeak(int band) {
  int xStart = BAR_WIDTH * band;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    matrix->drawPixel(x, peakHeight, ColorFromPalette(outrunPal, peakHeight * (255 / MATRIX_TILE_HEIGHT)));
  }
}

void waterfall(int band) {
  int xStart = BAR_WIDTH * band;
  double highestBandValue = 60000;        // Set this to calibrate your waterfall

  // Draw bottom line
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    matrix->drawPixel(x, 0, CHSV(constrain(map(bandValues[band],0,highestBandValue,160,0),0,160), 255, 255));
  }

  // Move screen up starting at 2nd row from top
  if (band == NUM_BANDS - 1){
    for (int y = MATRIX_HEIGHT - 2; y >= 0; y--) {
      for (int x = 0; x < MATRIX_WIDTH; x++) { // TODO: FIX THIS
        int pixelIndexY = matrix->XY(x, y + 1);
        int pixelIndex = matrix->XY(x, y);
        leds[pixelIndexY] = leds[pixelIndex];
      }
    }
  }
}