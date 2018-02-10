#include <Adafruit_NeoPixel.h>


#define ledLen 8
#define dataPin 8


Adafruit_NeoPixel rgb = Adafruit_NeoPixel(ledLen, dataPin, NEO_GRB + NEO_KHZ800);

uint32_t red = rgb.Color(255, 0, 0),
green = rgb.Color(0, 255, 0),
blue = rgb.Color(0, 0, 255),
white = rgb.Color(255, 255, 255);

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
  // put your setup code here, to run once:
  rgb.begin();
  rgb.setBrightness(100);
  rgb.clear();
  rgb.show();
}

void loop() {
  rgb.clear();
  for (int i = 0; i < 8; i++) {
    if (digitArray[0][i]) {
      rgb.setPixelColor(i, red);
    }
  }
  rgb.show();
  delay(1000);

 rgb.clear();
 for (int i = 0; i < 8; i++) {
    if (digitArray[1][i]) {
      rgb.setPixelColor(i, red);
    }
  }
  rgb.show();
  delay(1000);

  rgb.clear();
  for (int i = 0; i < 8; i++) {
    if (digitArray[2][i]) {
      rgb.setPixelColor(i, red);
    }
  }
  rgb.show();
  delay(1000);

  rgb.clear();
  for (int i = 0; i < 8; i++) {
    if (digitArray[3][i]) {
      rgb.setPixelColor(i, red);
    }
  }
  rgb.show();
  delay(1000);

  rgb.clear();
  for (int i = 0; i < 8; i++) {
    if (digitArray[4][i]) {
      rgb.setPixelColor(i, red);
    }
  }
  rgb.show();
  delay(1000);

  rgb.clear();
  for (int i = 0; i < 8; i++) {
    if (digitArray[5][i]) {
      rgb.setPixelColor(i, red);
    }
  }
  rgb.show();
  delay(1000);

  rgb.clear();
  for (int i = 0; i < 8; i++) {
    if (digitArray[6][i]) {
      rgb.setPixelColor(i, red);
    }
  }
  rgb.show();
  delay(1000);

}



