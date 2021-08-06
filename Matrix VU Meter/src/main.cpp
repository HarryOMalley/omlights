// (Heavily) adapted from https://github.com/G6EJD/ESP32-8266-Audio-Spectrum-Display/blob/master/ESP32_Spectrum_Display_02.ino
// Adjusted to allow brightness changes on press+hold, Auto-cycle for 3 button presses within 2 seconds
// Edited to add Neomatrix support for easier compatibility with different layouts.
#define FASTLED_INTERNAL
#include <Arduino.h>
#include <Wire.h>
#include <FastLED_NeoMatrix.h>
#include <arduinoFFT.h>
#include <EasyButton.h>

#define SAMPLES         1024          // Must be a power of 2
#define SAMPLING_FREQ   40000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AMPLITUDE       1000        // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
#define AUDIO_IN_PIN    35            // Signal in on this pin
#define LED_PIN         5             // LED strip data
#define BTN_PIN         4             // Connect a push button to this pin to change patterns
#define LONG_PRESS_MS   200           // Number of ms to count as a long press
#define COLOR_ORDER     GRB           // If colours look wrong, play with this
#define CHIPSET         WS2812B       // LED strip type
#define MAX_MILLIAMPS   20000          // Careful with the amount of power here if running off USB port
const int BRIGHTNESS_SETTINGS[3] = {5, 100, 255};  // 3 Integer array for 3 brightness settings (based on pressing+holding BTN_PIN)
#define LED_VOLTS       5             // Usually 5 or 12
#define NUM_BANDS       66            // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
#define NOISE           500           // Used as a crude noise filter, values below this are ignore

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
uint8_t colorTimer = 0;

// FastLED_NeoMaxtrix - see https://github.com/marcmerlin/FastLED_NeoMatrix for Tiled Matrixes, Zig-Zag and so forth
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, MATRIX_TILE_H, MATRIX_TILE_V, 
  NEO_MATRIX_TOP        + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS       + NEO_MATRIX_ZIGZAG +
  NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS + NEO_TILE_PROGRESSIVE);

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

void setup() {
  initialiseArrays();
  // Serial.begin(115200);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, MAX_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS_SETTINGS[0]);
  FastLED.clear();

  modeBtn.begin();
  modeBtn.onPressed(changeMode);
  modeBtn.onPressedFor(LONG_PRESS_MS, brightnessButton);
  modeBtn.onSequence(3, 2000, startAutoMode);
  modeBtn.onSequence(5, 2000, brightnessOff);
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQ));
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

void loop() {

  // Don't clear screen if waterfall pattern, be sure to change this is you change the patterns / order
  if (buttonPushCounter != 5) FastLED.clear();

  modeBtn.read();

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
if(i < 1) bandValues[0] += (int)vReal[i];
if(i>1 && i <= 2) bandValues[1] += (int)vReal[i];
if(i>2 && i <= 3) bandValues[2] += (int)vReal[i];
if(i>3 && i <= 4) bandValues[3] += (int)vReal[i];
if(i>4 && i <= 5) bandValues[4] += (int)vReal[i];
if(i>5 && i <= 6) bandValues[5] += (int)vReal[i];
if(i>6 && i <= 7) bandValues[6] += (int)vReal[i];
if(i>7 && i <= 8) bandValues[7] += (int)vReal[i];
if(i>8 && i <= 9) bandValues[8] += (int)vReal[i];
if(i>9 && i <= 10) bandValues[9] += (int)vReal[i];
if(i>10 && i <= 11) bandValues[10] += (int)vReal[i];
if(i>11 && i <= 12) bandValues[11] += (int)vReal[i];
if(i>12 && i <= 13) bandValues[12] += (int)vReal[i];
if(i>13 && i <= 14) bandValues[13] += (int)vReal[i];
if(i>14 && i <= 15) bandValues[14] += (int)vReal[i];
if(i>15 && i <= 16) bandValues[15] += (int)vReal[i];
if(i>16 && i <= 17) bandValues[16] += (int)vReal[i];
if(i>17 && i <= 18) bandValues[17] += (int)vReal[i];
if(i>18 && i <= 19) bandValues[18] += (int)vReal[i];
if(i>19 && i <= 20) bandValues[19] += (int)vReal[i];
if(i>20 && i <= 21) bandValues[20] += (int)vReal[i];
if(i>21 && i <= 22) bandValues[21] += (int)vReal[i];
if(i>22 && i <= 23) bandValues[22] += (int)vReal[i];
if(i>23 && i <= 24) bandValues[23] += (int)vReal[i];
if(i>24 && i <= 25) bandValues[24] += (int)vReal[i];
if(i>25 && i <= 26) bandValues[25] += (int)vReal[i];
if(i>26 && i <= 27) bandValues[26] += (int)vReal[i];
if(i>27 && i <= 28) bandValues[27] += (int)vReal[i];
if(i>28 && i <= 29) bandValues[28] += (int)vReal[i];
if(i>29 && i <= 30) bandValues[29] += (int)vReal[i];
if(i>30 && i <= 31) bandValues[30] += (int)vReal[i];
if(i>31 && i <= 32) bandValues[31] += (int)vReal[i];
if(i>32 && i <= 33) bandValues[32] += (int)vReal[i];
if(i>33 && i <= 34) bandValues[33] += (int)vReal[i];
if(i>34 && i <= 35) bandValues[34] += (int)vReal[i];
if(i>35 && i <= 37) bandValues[35] += (int)vReal[i];
if(i>37 && i <= 40) bandValues[36] += (int)vReal[i];
if(i>40 && i <= 43) bandValues[37] += (int)vReal[i];
if(i>43 && i <= 47) bandValues[38] += (int)vReal[i];
if(i>47 && i <= 51) bandValues[39] += (int)vReal[i];
if(i>51 && i <= 55) bandValues[40] += (int)vReal[i];
if(i>55 && i <= 60) bandValues[41] += (int)vReal[i];
if(i>60 && i <= 65) bandValues[42] += (int)vReal[i];
if(i>65 && i <= 71) bandValues[43] += (int)vReal[i];
if(i>71 && i <= 77) bandValues[44] += (int)vReal[i];
if(i>77 && i <= 83) bandValues[45] += (int)vReal[i];
if(i>83 && i <= 90) bandValues[46] += (int)vReal[i];
if(i>90 && i <= 98) bandValues[47] += (int)vReal[i];
if(i>98 && i <= 106) bandValues[48] += (int)vReal[i];
if(i>106 && i <= 115) bandValues[49] += (int)vReal[i];
if(i>115 && i <= 125) bandValues[50] += (int)vReal[i];
if(i>125 && i <= 136) bandValues[51] += (int)vReal[i];
if(i>136 && i <= 147) bandValues[52] += (int)vReal[i];
if(i>147 && i <= 160) bandValues[53] += (int)vReal[i];
if(i>160 && i <= 174) bandValues[54] += (int)vReal[i];
if(i>174 && i <= 188) bandValues[55] += (int)vReal[i];
if(i>188 && i <= 205) bandValues[56] += (int)vReal[i];
if(i>205 && i <= 222) bandValues[57] += (int)vReal[i];
if(i>222 && i <= 241) bandValues[58] += (int)vReal[i];
if(i>241 && i <= 261) bandValues[59] += (int)vReal[i];
if(i>261 && i <= 284) bandValues[60] += (int)vReal[i];
if(i>284 && i <= 308) bandValues[61] += (int)vReal[i];
if(i>308 && i <= 334) bandValues[62] += (int)vReal[i];
if(i>334 && i <= 362) bandValues[63] += (int)vReal[i];
if(i>362 && i <= 393) bandValues[64] += (int)vReal[i];
if(i>393 && i <= 426) bandValues[65] += (int)vReal[i];


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