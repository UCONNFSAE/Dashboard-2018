#include <Adafruit_NeoPixel.h>

#define LED_segs 8
#define outputPin 1   //Temporary
#define readPin 2     //Temporary


Adafruit_NeoPixel gearPosDisp = Adafruit_NeoPixel(LED_segs, outputPin, NEO_GRB + NEO_KHZ800);
uint32_t 
blue = gearPosDisp.Color(0, 0, 20),
segArray[8][8] = { //{a, b, c, d, e, f, g, h}
                     {0, 0, 1, 0, 1, 0, 1, 0}, // n
                     {0, 1, 1, 0, 0, 0, 0, 0}, // 1          a
                     {1, 1, 0, 1, 1, 0, 1, 0}, // 2       f     b
                     {1, 1, 1, 1, 0, 0, 1, 0}, // 3          g
                     {0, 1, 1, 0, 0, 1, 1, 0}, // 4       e     c
                     {1, 0, 1, 1, 0, 1, 1, 0}, // 5          d     h
                     {1, 0, 1, 1, 1, 1, 1, 0}  // 6
                 };

int prevSig = 0;


void setup() {
  // put your setup code here, to run once:
  strip.begin()
  strip.setBrightness(100);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  sig = map(analogRead(readPin, 0, 1023, 0, 6);
  
  if (Sig != prevSig) {
    updateGearInd(sig);
    prevSig = sig;
  }
}

void updateGearInd(int inputSig) {
  
  for (int i = 0; i < LED_segs; ++i) {
    if (segArray[inputSig][i] == 1) {
      gearPosDisp.setPixelColor(i, blue);
    }
  }
}


