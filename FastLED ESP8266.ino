///////////////////////////////////////////////////////////////////////////
//
// Controlling WS2812B over WiFi using FASTled + UDP messages received from Processing.
// Free to use, modify and distribute.
// Dave Stone, Aug 2016.
//
///////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <FastLED.h>
#include <elapsedMillis.h>

// LED config
#define NUM_LEDS 14
#define DATA_PIN D1
struct CRGB leds[NUM_LEDS];
int mappedHue;
TBlendType    currentBlending;
#define COOLING  30 // 20 - 100
#define SPARKING 100 // 50 - 200
bool gReverseDirection = false;
#define FRAMES_PER_SECOND 15

// Ripple animation config.

int center = 0;                                               // Center of the current ripple.
int step = -1;                                                // -1 is the initializing step.
uint8_t myfade = 255;                                         // Starting brightness.
#define maxsteps 16                                           // Case statement wouldn't allow a variable.
int thisdelay = 60;

// PALETTES

///////////setup/////////////////////////////////////////////////////////////
// Gradient palette "Crystal_Ice_Palace_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/Skyblue2u/tn/Crystal_Ice_Palace.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( Crystal_Ice_Palace_gp ) {
  0, 242, 223, 252,
  51, 242, 223, 252,
  51, 227, 237, 235,
  102, 227, 237, 235,
  102, 188, 235, 226,
  153, 188, 235, 226,
  153, 128, 217, 192,
  204, 128, 217, 192,
  204,  92, 176, 137,
  255,  92, 176, 137
};


// Gradient palette "Deep_Skyblues_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/Skyblue2u/tn/Deep_Skyblues.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( Deep_Skyblues_gp ) {
  0,   1, 55, 140,
  51,   1, 55, 140,
  51,   1, 18, 96,
  102,   1, 18, 96,
  102,   1,  1, 56,
  153,   1,  1, 56,
  153,   1,  1, 25,
  204,   1,  1, 25,
  204,   1,  1,  7,
  255,   1,  1,  7
};


// Gradient palette "dire_side_of_blue_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/Skyblue2u/tn/dire_side_of_blue.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( dire_side_of_blue_gp ) {
  0,  12, 56, 108,
  51,  12, 56, 108,
  51,   4, 35, 77,
  102,   4, 35, 77,
  102,   1, 22, 56,
  153,   1, 22, 56,
  153,   1, 10, 32,
  204,   1, 10, 32,
  204,   1,  3,  9,
  255,   1,  3,  9
};


// Gradient palette "es_seadreams_12_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/sea_dreams/tn/es_seadreams_12.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_seadreams_12_gp ) {
  0,   1,  1,  4,
  68,   1, 14, 21,
  140,  17, 139, 180,
  178, 137, 235, 233,
  219, 184, 248, 247,
  255, 244, 233, 230
};


// Gradient palette "es_seadreams_03_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/sea_dreams/tn/es_seadreams_03.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_seadreams_03_gp ) {
  0,  30, 124, 180,
  84,  80, 164, 203,
  153, 163, 209, 226,
  216, 244, 244, 240,
  255, 244, 244, 240
};


// Gradient palette "es_seadreams_11_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/sea_dreams/tn/es_seadreams_11.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_seadreams_11_gp ) {
  0,   1,  1, 240,
  255,   1, 239, 230
};


// Gradient palette "es_seadreams_04_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/sea_dreams/tn/es_seadreams_04.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_seadreams_04_gp ) {
  0, 137, 229, 240,
  68,   2, 59, 119,
  158,  16, 133, 205,
  252,  95, 233, 245,
  255,  95, 233, 245
};


// Gradient palette "colombia_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/wkp/shadowxfox/tn/colombia.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 112 bytes of program space.

DEFINE_GRADIENT_PALETTE( colombia_gp ) {
  0,   0,  2, 14,
  28,   0,  2, 14,
  28,   0,  7, 25,
  56,   0,  7, 25,
  56,   0, 33, 71,
  84,   0, 33, 71,
  84,   0, 82, 147,
  99,   0, 82, 147,
  99,  22, 149, 255,
  107,  22, 149, 255,
  107, 132, 215, 255,
  113, 132, 215, 255,
  113,  61, 105, 49,
  116,  61, 105, 49,
  116,  91, 135, 55,
  119,  91, 135, 55,
  119, 117, 156, 67,
  127, 117, 156, 67,
  127, 190, 189, 99,
  141, 190, 189, 99,
  141, 194, 152, 65,
  170, 194, 152, 65,
  170, 144, 105, 48,
  198, 144, 105, 48,
  198, 104, 81, 45,
  226, 104, 81, 45,
  226,  69, 51, 23,
  255,  69, 51, 23
};


// Gradient palette "OS_WAT_Mars_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/wesson/tn/OS_WAT_Mars.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 184 bytes of program space.

DEFINE_GRADIENT_PALETTE( OS_WAT_Mars_gp ) {
  0, 194, 229, 212,
  5, 194, 229, 207,
  11, 197, 227, 205,
  17, 197, 227, 203,
  22, 199, 227, 205,
  28, 199, 227, 205,
  34, 201, 229, 203,
  40, 206, 231, 203,
  45, 210, 233, 205,
  51, 213, 233, 203,
  57, 215, 233, 199,
  63, 220, 235, 199,
  68, 224, 233, 199,
  74, 232, 237, 199,
  80, 237, 237, 197,
  85, 237, 237, 197,
  91, 234, 237, 194,
  97, 234, 237, 194,
  103, 234, 237, 194,
  108, 232, 235, 190,
  114, 232, 229, 182,
  120, 229, 223, 176,
  126, 227, 219, 170,
  131, 229, 215, 170,
  137, 232, 211, 168,
  143, 234, 207, 164,
  148, 232, 197, 158,
  154, 224, 187, 149,
  160, 217, 178, 140,
  166, 208, 169, 132,
  171, 201, 164, 127,
  177, 197, 157, 120,
  183, 194, 154, 115,
  189, 182, 141, 106,
  194, 161, 125, 93,
  200, 144, 111, 84,
  206, 130, 99, 74,
  211, 115, 88, 66,
  217, 100, 77, 57,
  223,  92, 75, 57,
  229, 109, 93, 77,
  234, 132, 114, 98,
  240, 155, 136, 122,
  246, 182, 161, 149,
  252, 213, 189, 182,
  255, 227, 207, 199
};


// Gradient palette "es_skywalker_23_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/skywalker/tn/es_skywalker_23.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_skywalker_23_gp ) {
  0, 109, 154, 180,
  38,  67, 131, 180,
  109,  17, 75, 151,
  255,   0,  0,  1
};


// Gradient palette "es_skywalker_33_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/skywalker/tn/es_skywalker_33.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_skywalker_33_gp ) {
  0, 103, 239, 247,
  114,   1, 149, 223,
  242,   1, 43, 82,
  255,   1, 43, 82
};


// Gradient palette "es_landscape_22_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_22.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_landscape_22_gp ) {
  0,   1,  6,  1,
  38,   7, 49,  1,
  63,  21, 124,  1,
  68, 173, 244, 252,
  127,  10, 164, 156,
  255,   5, 68, 66
};


// Gradient palette "es_landscape_58_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_58.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_landscape_58_gp ) {
  0,   1, 100, 11,
  38,  15, 114,  6,
  63,  37, 128,  6,
  89,  68, 147,  3,
  107,  86, 147, 43,
  127, 106, 146, 151,
  178, 123, 166, 199,
  255, 153, 187, 216
};


// Gradient palette "es_landscape_65_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_65.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_landscape_65_gp ) {
  0,   0,  0,  0,
  31,   2, 25,  1,
  63,  15, 115,  5,
  127,  79, 213,  1,
  130, 126, 211, 47,
  132, 188, 209, 247,
  204,   2, 40, 33,
  255,   0,  1,  2
};


// Gradient palette "sky_45_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/rafi/tn/sky-45.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( sky_45_gp ) {

  0, 249, 205,  4,
  51, 255, 239, 123,
  87,   5, 141, 85,
  178,   1, 26, 43,
  255,   0,  2, 23
};


// Gradient palette "art_nouveau_02_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pd/art/tn/art-nouveau-02.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( art_nouveau_02_gp ) {
  0,  91, 50, 17,
  42,  91, 55, 30,
  84,  91, 59, 48,
  127,  63, 41, 33,
  170,  39, 26, 21,
  212,  46, 16, 11,
  255,  54,  9,  5
};


// Gradient palette "art_deco_05_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pd/art/tn/art-deco-05.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( art_deco_05_gp ) {
  0, 157, 149, 128,
  63,  97, 154, 142,
  127,  53, 157, 158,
  191,  46, 96, 87,
  255,  39, 51, 39
};


// Gradient palette "art_deco_02_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pd/art/tn/art-deco-02.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( art_deco_02_gp ) {
  0,  58, 56, 29,
  42,  83, 80, 39,
  84, 115, 109, 52,
  127,  73, 69, 59,
  177,  43, 37, 65,
  231,  22, 13, 42,
  255,  10,  1, 25
};


// Gradient palette "art_nouveau_04_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pd/art/tn/art-nouveau-04.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( art_nouveau_04_gp ) {
  0,  39, 26, 21,
  42,  48, 40, 25,
  84,  58, 56, 29,
  127, 115, 100, 42,
  170, 197, 159, 60,
  212, 112, 144, 74,
  255,  54, 131, 89
};


// Gradient palette "art_nouveau_05_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pd/art/tn/art-nouveau-05.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( art_nouveau_05_gp ) {
  0,  55, 41, 11,
  42,  82, 80, 29,
  84, 117, 133, 60,
  127,  83, 111, 55,
  170,  57, 91, 52,
  212,  48, 70, 52,
  255,  40, 51, 51
};


// Gradient palette "saga_09_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/saga/tn/saga-09.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 80 bytes of program space.

DEFINE_GRADIENT_PALETTE( saga_09_gp ) {
  0, 255, 255, 255,
  13, 220, 225, 245,
  26, 190, 199, 237,
  40, 161, 173, 228,
  53, 137, 151, 221,
  67, 113, 128, 214,
  80,  94, 109, 205,
  93,  77, 92, 199,
  107,  60, 75, 192,
  120,  47, 61, 184,
  134,  35, 48, 178,
  147,  26, 37, 170,
  161,  18, 27, 164,
  174,  12, 19, 158,
  187,   7, 13, 151,
  201,   4,  8, 145,
  214,   2,  4, 140,
  228,   1,  1, 133,
  241,   1,  1, 128,
  255,   0,  0, 123
};


// Gradient palette "saga_10_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/saga/tn/saga-10.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 80 bytes of program space.

DEFINE_GRADIENT_PALETTE( saga_10_gp ) {
  0, 255, 255,  0,
  13, 244, 225,  0,
  26, 237, 199,  0,
  40, 227, 173,  0,
  53, 220, 151,  0,
  67, 213, 128,  0,
  80, 203, 109,  0,
  93, 197, 92,  0,
  107, 190, 75,  0,
  120, 182, 61,  0,
  134, 175, 48,  0,
  147, 167, 37,  0,
  161, 161, 27,  0,
  174, 155, 19,  0,
  187, 148, 13,  0,
  201, 142,  8,  0,
  214, 137,  4,  0,
  228, 130,  1, 0,
  241, 125,  1,  0,
  255, 120,  0,  0
};



// Gradient palette "saga_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/saga/tn/saga-15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 80 bytes of program space.

DEFINE_GRADIENT_PALETTE( saga_15_gp ) {
  0,   0,  0, 255,
  13,   0,  1, 221,
  26,   0,  1, 192,
  40,   0,  4, 164,
  53,   0,  8, 140,
  67,   0, 13, 117,
  80,   0, 19, 98,
  93,   0, 27, 80,
  107,   0, 37, 64,
  120,   0, 48, 51,
  134,   0, 61, 38,
  147,  0, 75, 29,
  161,   0, 92, 20,
  174,   0, 109, 14,
  187,   0, 128,  9,
  201,   0, 151,  5,
  214,   0, 173,  2,
  228,   0, 199,  1,
  241,   0, 225,  1,
  255,   0, 255,  0
};




// Gradient palette "weather_the_blues_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/Skyblue2u/tn/weather_the_blues.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( weather_the_blues_gp ) {
  0,  41, 77, 100,
  51,  41, 77, 100,
  51,  16, 54, 100,
  102,  16, 54, 100,
  102,  13, 40, 62,
  153,  13, 40, 62,
  153,  51, 45, 22,
  204,  51, 45, 22,
  204,  97, 75, 38,
  255,  97, 75, 38
};

// Gradient palette "Adrift_in_Dreams_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/Skyblue2u/tn/Adrift_in_Dreams.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( Adrift_in_Dreams_gp ) {
  0, 148, 223, 77,
  51, 148, 223, 77,
  51,  86, 182, 89,
  102,  86, 182, 89,
  102,  36, 131, 72,
  153,  36, 131, 72,
  153,   5, 61, 51,
  204,   5, 61, 51,
  204,   1, 15, 29,
  255,   1, 15, 29
};


// Gradient palette "you_cheater_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/lightningmccarl/tn/you_cheater.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( you_cheater_gp ) {
  0, 255, 235, 252,
  43, 255, 235, 252,
  43, 237, 135, 103,
  53, 237, 135, 103,
  53, 217, 62, 25,
  71, 217, 62, 25,
  71, 123, 20, 14,
  130, 123, 20, 14,
  130,  58,  1,  6,
  255,  58,  1,  6
};

// Gradient palette "Run_Away_Tonight_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/electroluv/tn/Run_Away_Tonight.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( Run_Away_Tonight_gp ) {
  0, 179, 85, 38,
  76, 179, 85, 38,
  76, 101, 56, 38,
  104, 101, 56, 38,
  104,  29, 31, 32,
  117,  29, 31, 32,
  117,   5,  9, 11,
  135,   5,  9, 11,
  135,   1,  1,  2,
  255,   1,  1,  2
};


// Gradient palette "nrwc_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/wkp/tubs/tn/nrwc.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

DEFINE_GRADIENT_PALETTE( nrwc_gp ) {

  0,   1,  1,  1,
  25,   4,  8,  1,
  51,   1, 11,  2,
  76,   4, 36,  9,
  102,   6, 66, 18,
  127,  27, 95, 23,
  153,  82, 127, 31,
  178, 197, 171, 40,
  204, 133, 100, 19,
  229,  97, 48,  6,
  255, 163, 55,  7
};

// Gradient palette "es_ocean_breeze_068_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/ocean_breeze/tn/es_ocean_breeze_068.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_ocean_breeze_068_gp ) {
  0, 100, 156, 153,
  51,   1, 99, 137,
  101,   1, 68, 84,
  104,  35, 142, 168,
  178,   0, 63, 117,
  255,   1, 10, 10
};

// Gradient palette "es_ocean_breeze_036_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/ocean_breeze/tn/es_ocean_breeze_036.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_ocean_breeze_036_gp ) {
  0,   1,  6,  7,
  89,   1, 99, 111,
  153, 144, 209, 255,
  255,   0, 73, 82
};

// Gradient palette "lava_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/lava.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

DEFINE_GRADIENT_PALETTE( lava_gp ) {
  0,   0,  0,  0,
  46,  18,  0,  0,
  96, 113,  0,  0,
  108, 142,  3,  1,
  119, 175, 17,  1,
  146, 213, 44,  2,
  174, 255, 82,  4,
  188, 255, 115,  4,
  202, 255, 156,  4,
  218, 255, 203,  4,
  234, 255, 255,  4,
  244, 255, 255, 71,
  255, 255, 255, 255
};

// Gradient palette "fire_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/fire.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( fire_gp ) {
  0,   1,  1,  0,
  76,  32,  5,  0,
  146, 192, 24,  0,
  197, 220, 105,  5,
  240, 252, 255, 31,
  250, 252, 255, 111,
  255, 255, 255, 255
};

// Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( Sunset_Real_gp ) {
  0, 120,  0,  0,
  22, 179, 22,  0,
  51, 255, 104,  0,
  85, 167, 22, 18,
  135, 100,  0, 103,
  198,  16,  0, 130,
  255,   0,  0, 160
};

// Gradient palette "es_autumn_19_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/autumn/tn/es_autumn_19.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_autumn_19_gp ) {
  0,  26,  1,  1,
  51,  67,  4,  1,
  84, 118, 14,  1,
  104, 137, 152, 52,
  112, 113, 65,  1,
  122, 133, 149, 59,
  124, 137, 152, 52,
  135, 113, 65,  1,
  142, 139, 154, 46,
  163, 113, 13,  1,
  204,  55,  3,  1,
  249,  17,  1,  1,
  255,  17,  1,  1
};

// Gradient palette "es_emerald_dragon_08_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/emerald_dragon/tn/es_emerald_dragon_08.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_emerald_dragon_08_gp ) {
  0,  97, 255,  1,
  101,  47, 133,  1,
  178,  13, 43,  1,
  255,   2, 10,  1
};

DEFINE_GRADIENT_PALETTE( GMT_drywet_gp ) {
  0,  47, 30,  2,
  42, 213, 147, 24,
  84, 103, 219, 52,
  127,   3, 219, 207,
  170,   1, 48, 214,
  212,   1,  1, 111,
  255,   1,  7, 33
};

// Gradient palette "Coral_reef_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/other/tn/Coral_reef.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( Coral_reef_gp ) {
  0,  40, 199, 197,
  50,  10, 152, 155,
  96,   1, 111, 120,
  96,  43, 127, 162,
  139,  10, 73, 111,
  255,   1, 34, 71
};

// Gradient palette "es_rivendell_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/rivendell/tn/es_rivendell_15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_rivendell_15_gp ) {
  0,   1, 14,  5,
  101,  16, 36, 14,
  165,  56, 68, 30,
  242, 150, 156, 99,
  255, 150, 156, 99
};


// Zee's palettes.

DEFINE_GRADIENT_PALETTE( zee_one_gp ) {
  0,  74, 166, 171,
  3, 68, 156, 157,
  5,  67, 162, 156,
  10,  68, 157, 162,
  15,  33, 122, 136,
  20,  75, 167, 177,
  25,  71, 162, 173,
  30,  73, 167, 174,
  35,  84, 164, 158,
  40,  54, 115, 102,
  45,  12, 31, 17,
  50,  36, 121, 135,
  55,  34, 74, 54,
  60,  108, 164, 141,
  65,  63, 146, 150,
  70,  62, 156, 154,
  75,  54, 88, 58,
  80,  111, 104, 45,
  85,  86, 135, 102,
  90,  170, 140, 50,
  95,  123, 165, 129,
  100,  52, 140, 147,
  105,  34, 93, 88,
  110,  20, 20, 12,
  115,  8, 15, 17,
  120,  172, 140, 52,
  125,  211, 157, 52,
  130,  79, 104, 67,
  135,  64, 158, 165,
  140,  64, 140, 127,
  145,  56, 60, 28,
  150,  87, 57, 20,
  155,  216, 154, 46,
  160,  188, 148, 58,
  165,  186, 156, 67,
  170,  63, 149, 154,
  175,  55, 137, 147,
  180,  111, 99, 42,
  185,  210, 142, 36,
  190,  92, 80, 24,
  195,  157, 148, 74,
  200,  128, 107, 40,
  205,  66, 154, 159,
  210,  66, 157, 164,
  220,  9, 38, 34,
  230,  124, 112, 49,
  235,  93, 106, 53,
  245,  138, 158, 105,
  255,  124, 143, 87
};

DEFINE_GRADIENT_PALETTE( zee_two_gp ) {
  0, 123, 165, 129,
  50, 54, 88, 58,
  75, 124, 143, 87,
  100, 33, 122, 136,
  150, 73, 167, 174,
  175,  93, 106, 53,
  225,  211, 157, 52,
  255,  210, 142, 36
};

DEFINE_GRADIENT_PALETTE( zee_three_gp ) {
  0, 176, 163, 161,
  50, 117, 63, 88,
  75, 133, 50, 58,
  100, 17, 8, 41,
  150, 101, 21, 34,
  175,  166, 79, 65,
  225,  163, 129, 94,
  255,  174, 96, 34
};

//////////////////////////////////////////////
// PALETTE NAMES
//////////////////////////////////////////////
CRGBPalette16 earth = es_landscape_22_gp;
CRGBPalette16 dreams = Adrift_in_Dreams_gp;
CRGBPalette16 ice = Crystal_Ice_Palace_gp;
CRGBPalette16 sky = Deep_Skyblues_gp;
CRGBPalette16 direblue = dire_side_of_blue_gp;
CRGBPalette16 sea = es_seadreams_12_gp;
CRGBPalette16 sea2 = es_seadreams_03_gp;
CRGBPalette16 sea3 = es_seadreams_04_gp;
CRGBPalette16 colombia = colombia_gp;
CRGBPalette16 mars = OS_WAT_Mars_gp;
CRGBPalette16 nrwc = nrwc_gp;
CRGBPalette16 runaway = Run_Away_Tonight_gp;
CRGBPalette16 saga = saga_10_gp;
CRGBPalette16 nouveau = art_nouveau_05_gp;
CRGBPalette16 cheater = you_cheater_gp;
CRGBPalette16 weather = weather_the_blues_gp;
CRGBPalette16 ocean = es_ocean_breeze_068_gp;
CRGBPalette16 ocean2 = es_ocean_breeze_036_gp;
CRGBPalette16 lava = lava_gp;
CRGBPalette16 fire = fire_gp;
CRGBPalette16 sunset = Sunset_Real_gp;
CRGBPalette16 autumn = es_autumn_19_gp;
CRGBPalette16 dragon = es_emerald_dragon_08_gp;
CRGBPalette16 drywet = GMT_drywet_gp;
CRGBPalette16 coral = Coral_reef_gp;
CRGBPalette16 rivendell = es_rivendell_15_gp;
CRGBPalette16 zee = zee_one_gp;
CRGBPalette16 zee2 = zee_two_gp;
CRGBPalette16 zee3 = zee_three_gp;

// wifi connection variables
const char* ssid = ""; // name of network you want to connect to.
const char* password = ""; // password for above network.
boolean wifiConnected = false;

// UDP variables
unsigned int localPort = 8888;
WiFiUDP UDP;
boolean udpConnected = false;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet
unsigned int midiData[3]; // array for MIDI received over UDP (see accompnying Processing sketch)
int statusByte;
int param1;
int param2;

void setup() {

  wifiConnected = connectWifi();
  // only proceed if wifi connection successful
  if (wifiConnected) {
    udpConnected = connectUDP();
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  }
  for (int i = 0; i < NUM_LEDS; i++) { // initialize the LEDs as off.
    leds[i] = CRGB::Black;
    FastLED.show();
  }
  currentBlending = LINEARBLEND;
}

void loop() {

  random16_add_entropy( random(200));

  if (wifiConnected) {
    if (udpConnected) {

      // if there’s data available, read a packet
      int packetSize = UDP.parsePacket();
      if (packetSize)
      {
        UDP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        char *tmp; // the following code, using strtok, breaks up our packetBuffer at each space and fills out midiData array w/ the broken-up MIDI messages.
        int i = 0;
        tmp = strtok(packetBuffer, " "); // 'break message at space'
        while (tmp) {
          midiData[i++] = atoi(tmp);
          tmp = strtok(NULL, " ");
        }
        statusByte = midiData[0];
        param1 = midiData[1];
        param2 = midiData[2];
        for (int i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++) packetBuffer[i] = 0;
      }
    }
  }
  // the fun stuff. i have this setup for an AKAI apc40 mkII. first few sliders, first couple columns of buttons, first few knobs each do something different to our LEDs.
  if ( statusByte == 176 && param1 == 7 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      FastLED.setBrightness(map(param2, 0, 127, 0, 220));
      FastLED.show();
    }
  }
  else  if ( statusByte == 177 && param1 == 7 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Blue;
      FastLED.setBrightness(map(param2, 0, 127, 0, 220));
      FastLED.show();
    }
  }
  else  if ( statusByte == 178 && param1 == 7 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::ForestGreen;
      FastLED.setBrightness(map(param2, 0, 127, 0, 220));
      FastLED.show();
    }
  }
  else  if ( statusByte == 179 && param1 == 7 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::DarkRed;
      FastLED.setBrightness(map(param2, 0, 127, 0, 220));
      FastLED.show();
    }
  }
  else if ( statusByte == 176 && param1 == 48 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      mappedHue = map(param2, 0, 127, 0, 255);
      leds[i] = ColorFromPalette(zee3, mappedHue);
      FastLED.show();
    }
  }
  else if ( statusByte == 176 && param1 == 49 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      mappedHue = map(param2, 0, 127, 0, 255);
      leds[i] = ColorFromPalette(lava, mappedHue);
      FastLED.show();
    }
  }
  else if ( statusByte == 176 && param1 == 50 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      mappedHue = map(param2, 0, 127, 0, 255);
      leds[i] = ColorFromPalette(sea, mappedHue);
      FastLED.show();
    }
  }
  else if ( statusByte == 176 && param1 == 51 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      mappedHue = map(param2, 0, 127, 0, 255);
      leds[i] = ColorFromPalette(earth, mappedHue);
      FastLED.show();
    }
  }
  else if ( statusByte == 144 && param1 == 32 && param2 == 127 ) {
    Fire2012WithPalette();
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);
  }
  else if ( statusByte == 128 && param1 == 32 && param2 == 0 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      FastLED.show();
    }
  }
  else if ( statusByte == 144 && param1 == 24 && param2 == 127 ) {
    for (int i = 0; i < 220; i++) {
      FastLED.setBrightness(i);
      FastLED.show();
      delay(2);
    }
    for (int i = 220; i > 0; i--) {
      FastLED.setBrightness(i);
      FastLED.show();
      delay(2);
    }
  }
  else if ( statusByte == 144 && param1 == 16 && param2 == 127 ) {
    EVERY_N_MILLISECONDS(thisdelay) {                           // FastLED based non-blocking delay to update/display the sequence.
      ripple2();
    }
    FastLED.show();
  }
  else if ( statusByte == 144 && param1 == 8 && param2 == 127 ) {
    IceFireWithPalette();
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);
  }
  else if ( statusByte == 128 && param1 == 8 && param2 == 0 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      FastLED.show();
    }
  }
  else if ( statusByte == 144 && param1 == 0 && param2 == 127 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::White;
      FastLED.show();
    }
  }
  else if ( statusByte == 144 && param1 == 33 && param2 == 127 ) {
    {
      EVERY_N_MILLISECONDS(thisdelay) {                           // FastLED based non-blocking delay to update/display the sequence.
        ripple();
      }
      FastLED.show();
    }
  }
  else if ( statusByte == 144 && param1 == 25 && param2 == 127 ) {
    {
      EVERY_N_MILLISECONDS(thisdelay) {                           // FastLED based non-blocking delay to update/display the sequence.
        ripple3();
      }
      FastLED.show();
    }
  }
  else if ( statusByte == 144 && param1 == 17 && param2 == 127 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      FastLED.show();
    }
  }
  else if ( statusByte == 144 && param1 == 9 && param2 == 127 ) {
    {
      EVERY_N_MILLISECONDS(thisdelay) {                           // FastLED based non-blocking delay to update/display the sequence.
        ripple4();
      }
      FastLED.show();
    }
  }
    else if ( statusByte == 144 && param1 == 1 && param2 == 127 ) {
    LavaFireWithPalette();
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);
  }
    else if ( statusByte == 128 && param1 == 1 && param2 == 0 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      FastLED.show();
    }
  }
}

// connect to UDP – returns true if successful or false if not

boolean connectUDP() {
  boolean state = false;

  Serial.println("");
  Serial.println("Connecting to UDP");

  if (UDP.begin(localPort) == 1) {
    Serial.println("Connection successful");
    state = true;
  }
  else {
    Serial.println("Connection failed");
  }

  return state;
}

// connect to wifi – returns true if successful or false if not

boolean connectWifi() {
  boolean state = true;
  int i = 0;
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10) {
      state = false;
      break;
    }
    i++;
  }
  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  return state;
}

// animations. some franken-code from the FASTled examples and around the 'net.

void Fire2012WithPalette()
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( heat[j], 240);
    // rename palette below
    CRGB color = ColorFromPalette(zee, colorindex); // earth
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

// ripple functions.

void ripple() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(0, 0, 0);  // background colour.

  switch (step) {

    case -1:                                                          // Initialize ripple variables.
      center = random(NUM_LEDS);
      step = 0;
      break;

    case 0:
      leds[center] = CHSV(15, 255, 255);                          // Display the first pixel of the ripple.
      step ++;
      break;

    case maxsteps:                                                    // At the end of the ripples.
      step = -1;
      break;

    default:                                                             // Middle of the ripples.
      leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(0, 255, myfade / step * 2);   // Simple wrap from Marc Miller
      leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(15, 255, myfade / step * 2);
      step ++;                                                         // Next step.
      break;
  }
}


void ripple2() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(0, 0, 0);  // background colour.

  switch (step) {

    case -1:                                                          // Initialize ripple variables.
      center = random(NUM_LEDS);
      step = 0;
      break;

    case 0:
      leds[center] = CHSV(171, 255, 255);                          // Display the first pixel of the ripple.
      step ++;
      break;

    case maxsteps:                                                    // At the end of the ripples.
      step = -1;
      break;

    default:                                                             // Middle of the ripples.
      leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(171, 255, myfade / step * 2);   // Simple wrap from Marc Miller
      leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(128, 255, myfade / step * 2);
      step ++;                                                         // Next step.
      break;
  }
}

void ripple3() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(0, 0, 0);  // background colour.

  switch (step) {

    case -1:                                                          // Initialize ripple variables.
      center = random(NUM_LEDS);
      step = 0;
      break;

    case 0:
      leds[center] = CHSV(85, 255, 255);                          // Display the first pixel of the ripple.
      step ++;
      break;

    case maxsteps:                                                    // At the end of the ripples.
      step = -1;
      break;

    default:                                                             // Middle of the ripples.
      leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(80, 255, myfade / step * 2);   // Simple wrap from Marc Miller
      leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(75, 255, myfade / step * 2);
      step ++;                                                         // Next step.
      break;
  }
}

void ripple4() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(0, 0, 0);  // background colour.

  switch (step) {

    case -1:                                                          // Initialize ripple variables.
      center = random(NUM_LEDS);
      step = 0;
      break;

    case 0:
      leds[center] = CHSV(15, 0, 255);                          // Display the first pixel of the ripple.
      step ++;
      break;

    case maxsteps:                                                    // At the end of the ripples.
      step = -1;
      break;

    default:                                                             // Middle of the ripples.
      leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(0, 0, myfade / step * 2);   // Simple wrap from Marc Miller
      leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(15, 0, myfade / step * 2);
      step ++;                                                         // Next step.
      break;
  }
}

void IceFireWithPalette()
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( heat[j], 240);
    // rename palette below
    CRGB color = ColorFromPalette(sea, colorindex); // earth
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

void LavaFireWithPalette()
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( heat[j], 240);
    // rename palette below
    CRGB color = ColorFromPalette(lava, colorindex); // earth
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

