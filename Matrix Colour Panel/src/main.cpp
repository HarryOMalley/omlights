// (Heavily) adapted from
// https://github.com/G6EJD/ESP32-8266-Audio-Spectrum-Display/blob/master/ESP32_Spectrum_Display_02.ino
// Adjusted to allow brightness changes on press+hold, Auto-cycle for 3 button
// presses within 2 seconds Edited to add Neomatrix support for easier
// compatibility with different layouts.
#define FASTLED_INTERNAL
// #include "WiFi.h"
#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <EasyButton.h>
#include <FastLED_NeoMatrix.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <arduinoFFT.h>

const char *ssid = "Lima";
const char *password = "Limesaregreat";

const char *mqtt_server = "10.0.0.2";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

#define LED_PIN 14      // LED strip data
#define COLOR_ORDER GRB // If colours look wrong, play with this
#define CHIPSET WS2812B // LED strip type
#define MAX_MILLIAMPS                                                          \
  35000 // Careful with the amount of power here if running off USB port
const int BRIGHTNESS_SETTINGS[3] = {
    5, 100, 255};    // 3 Integer array for 3 brightness settings (based on
                     // pressing+holding BTN_PIN)
#define LED_VOLTS 12 // Usually 5 or 12

#define MATRIX_TILE_WIDTH 5 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT 5 // height of each matrix
#define MATRIX_TILE_H 1      // number of matrices arranged horizontally
#define MATRIX_TILE_V 1      // number of matrices arranged vertically

// Used by NeoMatrix
#define MATRIX_WIDTH (MATRIX_TILE_WIDTH * MATRIX_TILE_H)
#define MATRIX_HEIGHT (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define NUM_MATRIX (MATRIX_WIDTH * MATRIX_HEIGHT)

// Compat for some other demos
#define NUM_LEDS NUM_MATRIX

#define SERPENTINE                                                             \
  true // Set to false if your LEDS are connected end to end, true if serpentine

uint8_t mode = 0;
uint8_t strobeStatus = 0;
uint16_t strobeRate = 200;

// FastLED stuff
CRGB leds[NUM_LEDS];
DEFINE_GRADIENT_PALETTE(purple_gp){0,   0,   212, 255,     // blue
                                   255, 179, 0,   255};    // purple
DEFINE_GRADIENT_PALETTE(outrun_gp){0,   141, 0,   100,     // purple
                                   127, 255, 192, 0,       // yellow
                                   255, 0,   5,   255};    // blue
DEFINE_GRADIENT_PALETTE(greenblue_gp){0,   0, 255, 60,     // green
                                      64,  0, 236, 255,    // cyan
                                      128, 0, 5,   255,    // blue
                                      192, 0, 236, 255,    // cyan
                                      255, 0, 255, 60};    // green
DEFINE_GRADIENT_PALETTE(redyellow_gp){0,   200, 200, 200,  // white
                                      64,  255, 218, 0,    // yellow
                                      128, 231, 0,   0,    // red
                                      192, 255, 218, 0,    // yellow
                                      255, 200, 200, 200}; // white
CRGBPalette16 purplePal = purple_gp;
CRGBPalette16 outrunPal = outrun_gp;
CRGBPalette16 greenbluePal = greenblue_gp;
CRGBPalette16 heatPal = redyellow_gp;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PALETTES

// Gradient palette "YlOrBr_03_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cb/seq/tn/YlOrBr_03.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE(YlOrBr_03_gp){0,   255, 237, 119, 84,  255, 237, 119,
                                      84,  252, 142, 13,  170, 252, 142, 13,
                                      170, 167, 29,  1,   255, 167, 29,  1};

// Gradient palette "RdPu_03_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cb/seq/tn/RdPu_03.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE(RdPu_03_gp){0,   249, 191, 178, 84,  249, 191, 178,
                                    84,  242, 90,  108, 170, 242, 90,  108,
                                    170, 130, 1,   54,  255, 130, 1,   54};

// Gradient palette "BuGn_03_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cb/seq/tn/BuGn_03.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE(BuGn_03_gp){0,   192, 233, 240, 84,  192, 233, 240,
                                    84,  67,  176, 140, 170, 67,  176, 140,
                                    170, 2,   93,  21,  255, 2,   93,  21};

// Gradient palette "GMT_panoply_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gmt/tn/GMT_panoply.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 128 bytes of program space.

DEFINE_GRADIENT_PALETTE(GMT_panoply_gp){
    0,   1,   1,   168, 15,  1,   1,   168, 15,  1,   19,  255, 31,  1,   19,
    255, 31,  7,   79,  255, 47,  7,   79,  255, 47,  27,  138, 255, 63,  27,
    138, 255, 63,  47,  178, 255, 79,  47,  178, 255, 79,  71,  219, 255, 95,
    71,  219, 255, 95,  95,  233, 255, 111, 95,  233, 255, 111, 146, 255, 255,
    127, 146, 255, 255, 127, 255, 252, 10,  143, 255, 252, 10,  143, 255, 213,
    0,   159, 255, 213, 0,   159, 255, 142, 0,   175, 255, 142, 0,   175, 255,
    72,  0,   191, 255, 72,  0,   191, 255, 15,  0,   207, 255, 15,  0,   207,
    255, 0,   0,   223, 255, 0,   0,   223, 159, 0,   0,   239, 159, 0,   0,
    239, 73,  0,   0,   255, 73,  0,   0};

// Gradient palette "pm3d07_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pm/tn/pm3d07.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE(pm3d07_gp){0,   0,   255, 0, 84,  0,   32,  0,
                                   84,  255, 255, 0, 170, 135, 149, 0,
                                   170, 255, 0,   0, 255, 52,  0,   0};

// Gradient palette "Life_is_beautiful_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/katiekat013/tn/Life_is_beautiful.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE(Life_is_beautiful_gp){
    0,   2,   37,  16, 51,  2,   37,  16, 51,  3,   62,  11,  102, 3,
    62,  11,  102, 42, 68,  6,   153, 42, 68,  6,   153, 109, 112, 2,
    204, 109, 112, 2,  204, 242, 100, 0,  255, 242, 100, 0};

// Gradient palette "min_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/esdb/tn/min.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE(min_gp){0,   255, 255, 0,   84,  255, 255, 0,
                                84,  101, 255, 45,  170, 101, 255, 45,
                                170, 42,  255, 255, 255, 42,  255, 255};

// Gradient palette "seismic_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gery/tn/seismic.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 216 bytes of program space.

DEFINE_GRADIENT_PALETTE(seismic_gp){
    0,   255, 4,   2,   40,  255, 4,   2,   40,  255, 34,  26,  61,  255, 34,
    26,  61,  255, 66,  54,  75,  255, 66,  54,  75,  255, 92,  80,  85,  255,
    92,  80,  85,  255, 115, 103, 92,  255, 115, 103, 92,  255, 135, 123, 98,
    255, 135, 123, 98,  255, 154, 144, 103, 255, 154, 144, 103, 255, 169, 160,
    108, 255, 169, 160, 108, 255, 186, 178, 112, 255, 186, 178, 112, 255, 201,
    194, 116, 255, 201, 194, 116, 255, 217, 212, 120, 255, 217, 212, 120, 255,
    231, 228, 123, 255, 231, 228, 123, 255, 246, 245, 127, 255, 246, 245, 127,
    255, 255, 255, 127, 255, 255, 255, 127, 247, 248, 255, 130, 247, 248, 255,
    130, 229, 233, 255, 134, 229, 233, 255, 134, 210, 217, 255, 138, 210, 217,
    255, 138, 194, 203, 255, 142, 194, 203, 255, 142, 177, 187, 255, 146, 177,
    187, 255, 146, 159, 171, 255, 150, 159, 171, 255, 150, 140, 154, 255, 156,
    140, 154, 255, 156, 121, 136, 255, 162, 121, 136, 255, 162, 100, 115, 255,
    169, 100, 115, 255, 169, 77,  92,  255, 179, 77,  92,  255, 179, 50,  65,
    255, 193, 50,  65,  255, 193, 23,  33,  255, 214, 23,  33,  255, 214, 2,
    4,   255, 255, 2,   4,   255};

// Gradient palette "Romanian_flag_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ggr/tn/Romanian_flag.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

DEFINE_GRADIENT_PALETTE(Romanian_flag_gp){
    0,   0,   0,   255, 42,  0,   0,   255, 84,  0,   0,   255,
    84,  255, 255, 0,   127, 255, 255, 0,   170, 255, 255, 0,
    170, 255, 0,   0,   212, 255, 0,   0,   255, 255, 0,   0};

// Gradient palette "cmy_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/imagej/tn/cmy.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 1052 bytes of program space.

DEFINE_GRADIENT_PALETTE(cmy_gp){
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   255, 255, 1,   0,   255,
    255, 2,   0,   255, 255, 3,   0,   255, 255, 4,   0,   255, 255, 5,   0,
    255, 255, 6,   0,   255, 255, 7,   0,   255, 255, 8,   0,   255, 255, 9,
    0,   255, 255, 10,  0,   255, 255, 11,  0,   255, 255, 12,  0,   255, 255,
    13,  0,   255, 255, 14,  0,   255, 255, 15,  0,   255, 255, 16,  0,   255,
    255, 17,  0,   255, 255, 18,  0,   255, 255, 19,  0,   255, 255, 20,  0,
    255, 255, 21,  0,   255, 255, 22,  0,   255, 255, 23,  0,   255, 255, 24,
    0,   255, 255, 25,  0,   255, 255, 26,  0,   255, 255, 27,  0,   255, 255,
    28,  0,   255, 255, 29,  0,   255, 255, 30,  0,   255, 255, 31,  0,   255,
    255, 32,  0,   255, 255, 33,  0,   255, 255, 34,  0,   255, 255, 35,  0,
    255, 255, 210, 255, 255, 0,   211, 255, 255, 0,   212, 255, 255, 0,   213,
    255, 255, 0,   214, 255, 255, 0,   215, 255, 255, 0,   216, 255, 255, 0,
    217, 255, 255, 0,   218, 255, 255, 0,   219, 255, 255, 0,   220, 255, 255,
    0,   221, 255, 255, 0,   222, 255, 255, 0,   223, 255, 255, 0,   224, 255,
    255, 0,   225, 255, 255, 0,   226, 255, 255, 0,   227, 255, 255, 0,   228,
    255, 255, 0,   229, 255, 255, 0,   230, 255, 255, 0,   231, 255, 255, 0,
    232, 255, 255, 0,   233, 255, 255, 0,   234, 255, 255, 0,   235, 255, 255,
    0,   236, 255, 255, 0,   237, 255, 255, 0,   238, 255, 255, 0,   239, 255,
    255, 0,   240, 255, 255, 0,   241, 255, 255, 0,   242, 255, 255, 0,   243,
    255, 255, 0,   244, 255, 255, 0,   245, 255, 255, 0,   246, 255, 255, 0,
    247, 255, 255, 0,   248, 255, 255, 0,   249, 255, 255, 0,   250, 255, 255,
    0,   251, 255, 255, 0,   252, 255, 255, 0,   252, 217, 223, 219, 253, 217,
    223, 219, 253, 237, 239, 237, 254, 237, 239, 237, 254, 255, 255, 255, 255,
    255, 255, 255};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PALETTES END

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

CRGBPalette16 paletteList[] = {palette1, palette2, palette3, palette4,
                               palette5, palette6, palette7, palette8,
                               palette9, palette10};

int paletteIndex = 0;
CRGBPalette16 currentPalette = paletteList[paletteIndex];
CRGBPalette16 targetPalette = paletteList[paletteIndex];

uint8_t colorTimer = 0;

// FastLED_NeoMaxtrix - see https://github.com/marcmerlin/FastLED_NeoMatrix for
// Tiled Matrixes, Zig-Zag and so forth
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(
    leds, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, MATRIX_TILE_H, MATRIX_TILE_V,
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG +
        NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS + NEO_TILE_PROGRESSIVE);

void reconnect();
void changeMode();
void brightnessOff();
void mqttCallback(char *topic, byte *message, unsigned int length);
void paintWhite();
void paintBlack();

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalSMD5050);
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
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Matrix")) {
      Serial.println("Connected to MQTT Server");
      Serial.println("Subscribing to topics");
      // Subscribe
      client.subscribe("matrix/brightness");
      client.subscribe("strobe");
      client.subscribe("strobe/rate");
      client.subscribe("changePalette");
      client.subscribe("setPalette");
      Serial.println("Subscribed to topics");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttCallback(char *topicIn, byte *message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topicIn);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Changes the output state according to the message
  String topic = String(topicIn);
  if (topic == "matrix/brightness") {
    // Serial.print("Changing output to ");
    if (messageTemp == "2") {
      Serial.println("on");
      FastLED.setBrightness(BRIGHTNESS_SETTINGS[2]);
    } else if (messageTemp == "1") {
      Serial.println("on");
      FastLED.setBrightness(BRIGHTNESS_SETTINGS[1]);
    } else if (messageTemp == "0") {
      Serial.println("on");
      FastLED.setBrightness(BRIGHTNESS_SETTINGS[0]);
    } else if (messageTemp == "off") {
      Serial.println("off");
      FastLED.setBrightness(0);
    } else {
      FastLED.setBrightness(messageTemp.toInt());
    }
  }

  else if (topic == "strobe") {
    if (messageTemp == "true" || messageTemp == "on") {
      mode = 1;
    } else {
      mode = 0;
    }
  } else if (topic == "strobe/rate") {
    strobeRate = messageTemp.toInt();
  } else if (topic == "changePalette") {
    if (paletteIndex > 9) {
      paletteIndex = 0;
    } else {
      paletteIndex += 1;
    }

    targetPalette = paletteList[paletteIndex];
  }

  else if (topic == "setPalette") {
    int index = messageTemp.toInt();
    if (index > -1 && index <= 9) {
      paletteIndex = index;
      targetPalette = paletteList[paletteIndex];
    }
  } else {
    Serial.println("Received unknown message, ignoring");
  }
}

void changeMode() {
  // Serial.println("Button pressed");
  if (FastLED.getBrightness() == 0)
    FastLED.setBrightness(
        BRIGHTNESS_SETTINGS[0]); // Re-enable if lights are "off"
}

void brightnessButton() {
  if (FastLED.getBrightness() == BRIGHTNESS_SETTINGS[2])
    FastLED.setBrightness(BRIGHTNESS_SETTINGS[0]);
  else if (FastLED.getBrightness() == BRIGHTNESS_SETTINGS[0])
    FastLED.setBrightness(BRIGHTNESS_SETTINGS[1]);
  else if (FastLED.getBrightness() == BRIGHTNESS_SETTINGS[1])
    FastLED.setBrightness(BRIGHTNESS_SETTINGS[2]);
  else if (FastLED.getBrightness() == 0)
    FastLED.setBrightness(
        BRIGHTNESS_SETTINGS[0]); // Re-enable if lights are "off"
}

void brightnessOff() {
  FastLED.setBrightness(0); // Lights out
}

/********************************
                   MAIN LOOP
**************************************/
void loop() {

  if (!client.connected()) {
    Serial.println("Not connected anymore");
    reconnect();
  }
  client.loop();
  // esp_task_wdt_reset();
  // Serial.println("TEST");
  if (mode == 1) { // STROBE
    delay(strobeRate);
    if (strobeStatus == 0) {
      paintWhite();
      FastLED.show();
      strobeStatus = 1;
    } else {
      paintBlack();
      FastLED.show();
      strobeStatus = 0;
    }
  } else {

    EVERY_N_MILLISECONDS(10) {
      nblendPaletteTowardPalette(currentPalette, targetPalette, 24);
    }

    FastLED.clear();

    // Used in some of the patterns
    EVERY_N_MILLISECONDS(10) { colorTimer++; }

    FastLED.show();
  }
}

// PATTERNS BELOW //

void paintWhite() {
  // matrix->fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, LED_WHITE_HIGH);
  for (int i = 0; i < MATRIX_WIDTH; i++) {
    for (int j = 0; j < MATRIX_HEIGHT; j++) {
      matrix->drawPixel(i, j, CRGB(255, 255, 255));
    }
  }
}

void paintBlack() {
  // matrix->fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, LED_BLACK);
  for (int i = 0; i < MATRIX_WIDTH; i++) {
    for (int j = 0; j < MATRIX_HEIGHT; j++) {
      matrix->drawPixel(i, j, CRGB(0, 0, 0));
    }
  }
}
