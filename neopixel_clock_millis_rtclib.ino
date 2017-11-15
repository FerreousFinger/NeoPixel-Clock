#include <Time.h>
#include <Wire.h>
#include "RTClib.h"

#include <Adafruit_NeoPixel.h>

#define NEO_PIN 7
#define LEDS 60
#define DELAYVAL 250
#define PHOTOCELL 0
#define COMPUTER_MAX_ATTEMPTS 6
#define COMPUTER_TIMEOUT 10

bool computerMode = false;
int computerAttempts = 0;
bool errorMode = false;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);
RTC_DS1307 rtc;

uint32_t colorHours = pixels.Color(255, 0, 0);
uint32_t colorHoursLow = pixels.Color(60, 0, 0);
uint32_t colorMinutes = pixels.Color(0, 0, 200);
uint32_t colorMinutesLow = pixels.Color(0, 0, 20);
uint32_t colorSeconds = pixels.Color(0, 255, 0);
uint32_t colorSecondsLow = pixels.Color(0, 20, 0);
uint32_t colorClear = pixels.Color(0, 0, 0);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(57600);
  pixels.begin();
  if(!rtc.begin()) {
    errorMode = true;
  }

  if(!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //if(Serial)
  //{
  //  Serial.print('Computer connected');
  //  computerMode = true;
  //}
  
  clearPixels();
  initialize();
}

void loop() {
  // put your main code here, to run repeatedly
  if(computerMode) {
    //TODO: Add clock update logic here
    return;
  }
  
  if(rtc.isrunning()) {

    int h = getHours();
    if(h > 11)
      h -= 12;
    h *= 5;
  
    int m = getMinutes();
    int s = getSeconds();

    //printTimeToSerial();

    if(m == 0 && s == 0)
      clearAnimation();

    //Clear
    setColorManually(colorClear);
    
    //Fill minutes
    for(int i = 0; i < m; i++)
      setColorManually(i, colorMinutes);
  
    //Set hours
    if(h > 0)
      setColorManually(h - 1, colorHoursLow);
    else
      setColorManually(pixels.numPixels() - 1, colorHoursLow);
      
    setColorManually(h, colorHours);
    setColorManually(h + 1, colorHoursLow);

    if(m == h - 1) {
      setColorManually(m, colorMinutes);
    } else {
      if(m == h) {
        setColorManually(m, colorMinutes);
      } else {
        if(m == h + 1) {
          setColorManually(m, colorMinutes); 
        }
      }
    }

    //Set seconds
    setColor(s, colorSeconds);
  } else {
    if(errorMode) {
      setColor(colorHours);
      delay(500);
      setColor(colorClear);
      delay(500);
    } else {
      setColor(colorSeconds);
      delay(500);
      setColor(colorClear);
      delay(500);
    }
  }
  
  adjustBrightness();
  delay(DELAYVAL);
}

/*void serialEvent()
{
  while(Serial.available())
  {
    String s = "";
    char input = (char)Serial.read();
    s += input;

    String t = getHours() + ":" + getMinutes() + ":" + getSeconds();
    if(s == "t")
      Serial.write(t);
  }
}*/

/*void adjustBrightness()
{
  int h = getHours();
  if(h >= 8 && h < 20)
    pixels.setBrightness(255);
  else if((h >= 20 && h < 23) || (h >= 6 && h < 8))
    pixels.setBrightness(192);
  else
    pixels.setBrightness(128);
}*/

void printTimeToSerial() {
  DateTime now = rtc.now();
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}

void adjustBrightness() {
  int photocellReading = analogRead(PHOTOCELL);
  int brightness = map(photocellReading, 0, 1023, 0, 120);//200
  pixels.setBrightness(brightness);
  pixels.show();
}

void clearAnimation() {
  setColor(pixels.numPixels() - 1, colorMinutes);

  int h = getHours();
  if(h > 11)
    h -= 12;
  h *= 5;
  
  for(int i = 0; i < pixels.numPixels(); i++) {
    if(i == h - 1 || i == h + 1) {
      setColor(i, colorHoursLow);
    } else if(i == h) {
      setColor(i, colorHours);
    } else {
      setColor(i, colorClear);
    }
    delay(50);
  }
  
  uint32_t prevColor = colorClear;
  for(int i = 0; i < getSeconds(); i++) {
    if(i > 0) {
      setColorManually(i - 1, prevColor);
    }
    
    prevColor = pixels.getPixelColor(i);
    setColor(i, colorSeconds);
    delay(50);
  }
//  initialize();
}

void initialize() {
  adjustBrightness();
  
  for(int i = 0; i < getMinutes(); i++) {
    setColor(i, colorMinutes);
    delay(50);
  }

  int h = getHours();
  if(h > 11)
    h -= 12;
  h *= 5;

  uint32_t prevColor = colorMinutes;
  for(int i = 0; i <= h + 1; i++) {      
    if(i > 0 && i < h) //Set i - 1 back to minute color, until i reaches hours //But this is fucking stupid...set it back to its previous color you dump ass!!
      setColorManually(i - 1, prevColor);

    uint32_t c = colorHours;
    if(i == h - 1 || i == h + 1)
      c = colorHoursLow;
    
    prevColor = pixels.getPixelColor(i);
    setColor(i, c);
    delay(50);
  }

  prevColor = colorMinutes;
  for(int i = 0; i < getSeconds(); i++) {
    if(i > 0) {
      setColorManually(i - 1, prevColor);
    }
    
    prevColor = pixels.getPixelColor(i);
    setColor(i, colorSeconds);
    delay(50);
  }
}

int getHours() {
  return rtc.now().hour();
}

int getMinutes() {
  return rtc.now().minute();
}

int getSeconds() {
  return rtc.now().second();
}

void setColor(uint32_t color) {
  for(int i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

void setColorManually(uint32_t color) {
  for(int i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color);
  }
}

void setColor(int led, uint32_t color) {
  pixels.setPixelColor(led, color);
  pixels.show();
}

void setColorManually(int led, uint32_t color) {
  pixels.setPixelColor(led, color);
}

void clearPixels() {
  setColor(colorClear);
}
