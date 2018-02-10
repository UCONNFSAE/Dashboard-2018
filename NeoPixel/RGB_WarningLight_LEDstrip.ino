#include <Adafruit_NeoPixel.h>
#include <Adafruit_TLC59711.h>
#include <SPI.h>`

#define numPWM 1
#define pwmClkPin 4
#define pwmDataPin 5
#define stripDataPin 6
#define rgbDataPin 7

#define rgbReadPin 9
#define stripReadPin 8

#define stripLength 16
#define rgbLength 8


Adafruit_TLC59711 pwm = Adafruit_TLC59711(numPWM, pwmClkPin, pwmDataPin);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(stripLength, stripDataPin, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel rgb = Adafruit_NeoPixel(rgbLength, rgbDataPin, NEO_GRB + NEO_KHZ800);

uint32_t redRGB = rgb.Color(255, 0, 0),
yellowRGB = rgb.Color(255, 255, 0),
greenRGB = rgb.Color(0, 255, 0),
blueRGB = rgb.Color(0, 0, 255),
whiteRGB = rgb.Color(255, 255, 255);

uint32_t greenStrip = strip.Color(0, 20, 0),
         yellowStrip = strip.Color(20, 20, 0),
         redStrip = strip.Color(20, 0, 0),
         blueStrip = strip.Color(0, 0, 20),
         ledArray[16] = {greenStrip, greenStrip, greenStrip, greenStrip, greenStrip, yellowStrip, yellowStrip, yellowStrip, yellowStrip, yellowStrip, yellowStrip, redStrip, redStrip, redStrip, redStrip, redStrip};



int digitArray[7][8] = {
                    {0, 0, 1, 0, 1, 0, 1, 0},  // n
                    {0, 1, 1, 0, 0, 0, 0, 0},  // 1
                    {1, 1, 0, 1, 1, 0, 1, 0},  // 2
                    {1, 1, 1, 1, 0, 0, 1, 0},  // 3
                    {0, 1, 1, 0, 0, 1, 1, 0},  // 4
                    {1, 0, 1, 1, 0, 1, 1, 0},  // 5
                    {1, 0, 1, 1, 1, 1, 1, 0}   // 6
                   };




void setup() {

  //PWM setup
  pwm.begin();
  pwm.write();
  pwm.setPWM(0, 32000);
  pwm.write();

  
  //LED strip setup
  strip.begin();
  strip.setBrightness(100);


  //RGB digit setup
  rgb.begin();
  rgb.setBrightness(100);
  Serial.begin(9600);

}

void loop() {

  int stripRead = map(analogRead(2), 0, 1023, 0, 16);
  int rgbRead = map(analogRead(0), 0, 1023, 0, 6);
  //Serial.println(rgbRead);
  Serial.println(stripRead);
  /*
  rgb.clear();
  rgb.show();
  
  // RGB digit - Gear Position
  for (int i = 0; i < 9; i++) {
    if (digitArray[rgbRead][i]) {
      rgb.setPixelColor(i, redRGB);
    }
  }
  rgb.show();

/*
  for (int j = 0; j < 16; j++) {
    strip.setPixelColor(j, redStrip);
  }
  strip.show();
  delay(1000);

  strip.clear();
  strip.show();
  delay(1000);

 */ 
}
