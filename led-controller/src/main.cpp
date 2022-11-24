#include "FastLED.h"
#include "WiFi.h"
#include <PubSubClient.h>
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif
const char *ssid = "Lima";
const char *password = "Limesaregreat";

const char *mqtt_server = "10.0.0.2";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
uint8_t mode = 0;
uint8_t strobeStatus = 0;
uint16_t strobeRate = 200;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///PALETTES

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
///PALETTES END

// CRGBPalette16 palette1 = YlOrBr_03_gp;
CRGBPalette16 palette2 = RdPu_03_gp;
CRGBPalette16 palette3 = BuGn_03_gp;
CRGBPalette16 palette4 = GMT_panoply_gp;
CRGBPalette16 palette5 = pm3d07_gp;
CRGBPalette16 palette6 = Life_is_beautiful_gp;
CRGBPalette16 palette7 = min_gp;
CRGBPalette16 palette8 = seismic_gp;
CRGBPalette16 palette9 = Romanian_flag_gp;
CRGBPalette16 palette10 = cmy_gp;

CRGBPalette16 paletteList[] = {palette2, palette3, palette4, palette5, palette6,
                               palette7, palette8, palette9, palette10};

int paletteIndex = 0;
CRGBPalette16 currentPalette = paletteList[paletteIndex];
CRGBPalette16 targetPalette = paletteList[paletteIndex];

#define NUM_LEDS 200
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define DATA_PIN 5
//#define CLK_PIN       4
#define VOLTS 5
#define MAX_MA 10000

void chooseNextColorPalette(CRGBPalette16 &);
void drawTwinkles(CRGBSet &);
CRGB computeOneTwinkle(uint32_t, uint8_t);
uint8_t attackDecayWave8(uint8_t);
void coolLikeIncandescent(CRGB &, uint8_t);

void reconnect();
void mqttCallback(char *topic, byte *message, unsigned int length);
void paintWhite();
void paintBlack();

CRGBArray<NUM_LEDS> leds;

// Overall twinkle speed.
// 0 (VERY slow) to 8 (VERY fast).
// 4, 5, and 6 are recommended, default is 4.
#define TWINKLE_SPEED 1

// Overall twinkle density.
// 0 (NONE lit) to 8 (ALL lit at once).
// Default is 5.
#define TWINKLE_DENSITY 8

// How often to change color palettes.
#define SECONDS_PER_PALETTE 30
// Also: toward the bottom of the file is an array
// called "ActivePaletteList" which controls which color
// palettes are used; you can add or remove color palettes
// from there freely.

// Background color for 'unlit' pixels
// Can be set to CRGB::Black if desired.
CRGB gBackgroundColor = CRGB::Black;
// Example of dim incandescent fairy light background color
// CRGB gBackgroundColor = CRGB(CRGB::FairyLight).nscale8_video(16);

// If AUTO_SELECT_BACKGROUND_COLOR is set to 1,
// then for any palette where the first two entries
// are the same, a dimmed version of that color will
// automatically be used as the background color.
#define AUTO_SELECT_BACKGROUND_COLOR 1

// If COOL_LIKE_INCANDESCENT is set to 1, colors will
// fade out slighted 'reddened', similar to how
// incandescent bulbs change color as they get dim down.
#define COOL_LIKE_INCANDESCENT 1

CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;

void setup() {

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);

  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MAX_MA);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);

  // chooseNextColorPalette(gTargetPalette);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
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

    EVERY_N_SECONDS(180) {
      if (paletteIndex > 9) {
        paletteIndex = 0;
      } else {
        paletteIndex += 1;
      }
      targetPalette = paletteList[paletteIndex];
    }

    EVERY_N_MILLISECONDS(10) {
      nblendPaletteTowardPalette(currentPalette, targetPalette, 12);
    }

    drawTwinkles(leds);

    FastLED.show();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
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

void mqttCallback(char *topic, byte *message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "strobe") {
    if (messageTemp == "on") {
      mode = 1;
    } else {
      mode = 0;
    }
  }
  if (String(topic) == "strobe/rate") {
    strobeRate = messageTemp.toInt();
  }
  if (String(topic) == "changePalette") {
    if (paletteIndex > 9) {
      paletteIndex = 0;
    } else {
      paletteIndex += 1;
    }

    targetPalette = paletteList[paletteIndex];
  }

  if (String(topic) == "setPalette") {
    int index = messageTemp.toInt();
    if (index > -1 && index <= 9) {
      paletteIndex = index;
      targetPalette = paletteList[paletteIndex];
    }
  }
}

void paintWhite() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255, 255, 255);
  }
}

void paintBlack() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
}

//  This function loops over each pixel, calculates the
//  adjusted 'clock' that this pixel should use, and calls
//  "CalculateOneTwinkle" on each pixel.  It then displays
//  either the twinkle color of the background color,
//  whichever is brighter.
void drawTwinkles(CRGBSet &L) {
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;

  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if AUTO_SELECT_BACKGROUND_COLOR == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg;
  if ((AUTO_SELECT_BACKGROUND_COLOR == 1) &&
      (gCurrentPalette[0] == gCurrentPalette[1])) {
    bg = gCurrentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if (bglight > 64) {
      bg.nscale8_video(16); // very bright, so scale to 1/16th
    } else if (bglight > 16) {
      bg.nscale8_video(64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video(86); // dim, scale to 1/3rd.
    }
  } else {
    bg = gBackgroundColor; // just use the explicitly defined background color
  }

  uint8_t backgroundBrightness = bg.getAverageLight();

  for (CRGB &pixel : L) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16 = PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to
    // 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =
        ((((PRNG16 & 0xFF) >> 4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 =
        (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle(myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if (deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color,
      // use the new color.
      pixel = c;
    } else if (deltabright > 0) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      pixel = blend(bg, c, deltabright * 8);
    } else {
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      pixel = bg;
    }
  }
}

//  This function takes a time in pseudo-milliseconds,
//  figures out brightness = f( time ), and also hue = f( time )
//  The 'low digits' of the millisecond time are used as
//  input to the brightness wave function.
//  The 'high digits' are used to select a color, so that the color
//  does not change over the course of the fade-in, fade-out
//  of one cycle of the brightness wave function.
//  The 'high digits' are also used to determine whether this pixel
//  should light at all during this cycle, based on the TWINKLE_DENSITY.
CRGB computeOneTwinkle(uint32_t ms, uint8_t salt) {
  uint16_t ticks = ms >> (8 - TWINKLE_SPEED);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8(slowcycle16);
  slowcycle16 = (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

  uint8_t bright = 0;
  if (((slowcycle8 & 0x0E) / 2) < TWINKLE_DENSITY) {
    bright = attackDecayWave8(fastcycle8);
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if (bright > 0) {
    c = ColorFromPalette(currentPalette, hue, bright, NOBLEND);
    if (COOL_LIKE_INCANDESCENT == 1) {
      coolLikeIncandescent(c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}

// This function is like 'triwave8', which produces a
// symmetrical up-and-down triangle sawtooth waveform, except that this
// function produces a triangle wave with a faster attack and a slower decay:
//
//     / \ 
//    /     \ 
//   /         \ 
//  /             \ 
//

uint8_t attackDecayWave8(uint8_t i) {
  if (i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i / 2));
  }
}

// This function takes a pixel, and if its in the 'fading down'
// part of the cycle, it adjusts the color a little bit like the
// way that incandescent bulbs fade toward 'red' as they dim.
void coolLikeIncandescent(CRGB &c, uint8_t phase) {
  if (phase < 128)
    return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8(c.g, cooling);
  c.b = qsub8(c.b, cooling * 2);
}

// // A mostly red palette with green accents and white trim.
// // "CRGB::Gray" is used as white to keep the brightness more uniform.
// const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM =
// {  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
//    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
//    CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray,
//    CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green };

// // A mostly (dark) green palette with red berries.
// #define Holly_Green 0x00580c
// #define Holly_Red   0xB00402
// const TProgmemRGBPalette16 Holly_p FL_PROGMEM =
// {  Holly_Green, Holly_Green, Holly_Green, Holly_Green,
//    Holly_Green, Holly_Green, Holly_Green, Holly_Green,
//    Holly_Green, Holly_Green, Holly_Green, Holly_Green,
//    Holly_Green, Holly_Green, Holly_Green, Holly_Red
// };

// // A red and white striped palette
// // "CRGB::Gray" is used as white to keep the brightness more uniform.
// const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM =
// {  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red,
//    CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
//    CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red,
//    CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// // A mostly blue palette with white accents.
// // "CRGB::Gray" is used as white to keep the brightness more uniform.
// const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM =
// {  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
//    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
//    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
//    CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// // A pure "fairy light" palette with some brightness variations
// #define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
// #define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
// const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM =
// {  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight,
//    HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight,
//    QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight,
//    CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight };

// // A palette of soft snowflakes with the occasional bright one
// const TProgmemRGBPalette16 Snow_p FL_PROGMEM =
// {  0x304048, 0x304048, 0x304048, 0x304048,
//    0x304048, 0x304048, 0x304048, 0x304048,
//    0x304048, 0x304048, 0x304048, 0x304048,
//    0x304048, 0x304048, 0x304048, 0xE0F0FF };

// // A palette reminiscent of large 'old-school' C9-size tree lights
// // in the five classic colors: red, orange, green, blue, and white.
// #define C9_Red    0xB80400
// #define C9_Orange 0x902C02
// #define C9_Green  0x046002
// #define C9_Blue   0x070758
// #define C9_White  0x606820
// const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM =
// {  C9_Red,    C9_Orange, C9_Red,    C9_Orange,
//    C9_Orange, C9_Red,    C9_Orange, C9_Red,
//    C9_Green,  C9_Green,  C9_Green,  C9_Green,
//    C9_Blue,   C9_Blue,   C9_Blue,
//    C9_White
// };

// // A cold, icy pale blue palette
// #define Ice_Blue1 0x0C1040
// #define Ice_Blue2 0x182080
// #define Ice_Blue3 0x5080C0
// const TProgmemRGBPalette16 Ice_p FL_PROGMEM =
// {
//   Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
//   Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
//   Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
//   Ice_Blue2, Ice_Blue2, Ice_Blue2, Ice_Blue3
// };

// // Add or remove palette names from this list to control which color
// // palettes are used, and in what order.
// const TProgmemRGBPalette16* ActivePaletteList[] = {
//   // &RetroC9_p,
//   // &BlueWhite_p,
//   &RainbowColors_p,
//   // &FairyLight_p,
//   // &RedGreenWhite_p,
//   // &PartyColors_p,
//   // &RedWhite_p,
//   // &Snow_p,
//   // &Holly_p,
//   // &Ice_p
// };

// // Advance to the next color palette in the list (above).
// void chooseNextColorPalette( CRGBPalette16& pal)
// {
//   const uint8_t numberOfPalettes = sizeof(ActivePaletteList) /
//   sizeof(ActivePaletteList[0]); static uint8_t whichPalette = -1;
//   whichPalette = addmod8( whichPalette, 1, numberOfPalettes);

//   pal = *(ActivePaletteList[whichPalette]);
// }