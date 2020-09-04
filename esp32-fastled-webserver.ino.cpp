# 1 "/tmp/tmp5asiwho2"
#include <Arduino.h>
# 1 "/home/Nicked/Documents/PlatformIO/Projects/200904-170622-esp32dev/src/esp32-fastled-webserver.ino"
# 25 "/home/Nicked/Documents/PlatformIO/Projects/200904-170622-esp32dev/src/esp32-fastled-webserver.ino"
#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <EEPROM.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3003000)
#warning "Requires FastLED 3.3 or later; check github for latest code."
#endif

WebServer webServer(80);

const int led = 5;

uint8_t autoplay = 0;
uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

uint8_t currentPatternIndex = 0;

uint8_t gHue = 0;

uint8_t power = 1;
uint8_t brightness = 8;

uint8_t speed = 30;




uint8_t cooling = 50;




uint8_t sparking = 120;

CRGB solidColor = CRGB::Blue;

uint8_t cyclePalettes = 0;
uint8_t paletteDuration = 10;
uint8_t currentPaletteIndex = 0;
unsigned long paletteTimeout = 0;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN 12

#define LED_TYPE WS2812B
#define COLOR_ORDER RGB
#define NUM_STRIPS 8
#define NUM_LEDS_PER_STRIP 100
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
CRGB leds[NUM_LEDS];

#define MILLI_AMPS 4000
#define FRAMES_PER_SECOND 120

#include "patterns.h"

#include "field.h"
#include "fields.h"

#include "secrets.h"
#include "wifi.h"
#include "web.h"







void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
void setup();
void loop();
void nextPattern();
void nextPalette();
#line 130 "/home/Nicked/Documents/PlatformIO/Projects/200904-170622-esp32dev/src/esp32-fastled-webserver.ino"
void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);


  Serial.begin(115200);

  SPIFFS.begin();
  listDir(SPIFFS, "/", 1);



  setupWifi();
  setupWeb();
# 152 "/home/Nicked/Documents/PlatformIO/Projects/200904-170622-esp32dev/src/esp32-fastled-webserver.ino"
  FastLED.addLeds<LED_TYPE, 13, COLOR_ORDER>(leds, 0, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 12, COLOR_ORDER>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 27, COLOR_ORDER>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 33, COLOR_ORDER>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 15, COLOR_ORDER>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 32, COLOR_ORDER>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 14, COLOR_ORDER>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, SCL, COLOR_ORDER>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);

  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);


  FastLED.setBrightness(brightness);

  autoPlayTimeout = millis() + (autoplayDuration * 1000);
}

void loop()
{
  handleWeb();

  if (power == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  else {

    patterns[currentPatternIndex].pattern();

    EVERY_N_MILLISECONDS(40) {

      nblendPaletteTowardPalette(currentPalette, targetPalette, 8);
      gHue++;
    }

    if (autoplay == 1 && (millis() > autoPlayTimeout)) {
      nextPattern();
      autoPlayTimeout = millis() + (autoplayDuration * 1000);
    }

    if (cyclePalettes == 1 && (millis() > paletteTimeout)) {
      nextPalette();
      paletteTimeout = millis() + (paletteDuration * 1000);
    }
  }


  FastLED.show();



  delay(1000 / FRAMES_PER_SECOND);
}

void nextPattern()
{

  currentPatternIndex = (currentPatternIndex + 1) % patternCount;
}

void nextPalette()
{
  currentPaletteIndex = (currentPaletteIndex + 1) % paletteCount;
  targetPalette = palettes[currentPaletteIndex];
}