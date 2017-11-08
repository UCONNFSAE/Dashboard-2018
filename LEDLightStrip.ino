#include <Adafruit_NeoPixel.h>

#define LED_len 16
#define PIN6 6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_len, PIN6, NEO_GRB + NEO_KHZ800);
uint32_t green = strip.Color(0, 20, 0),
         yellow = strip.Color(20, 20, 0),
         red = strip.Color(20, 0, 0),
         red_max = strip.Color(100, 0, 0),
         blue = strip.Color(0, 0, 20),
         ledArray[16] = {green, green, green, green, green, yellow, yellow, yellow, yellow, yellow, yellow, red, red, red, red, red};

int prev_RPM_Signal = 0;



void setup() {
  // put your setup code here, to run once:  
  strip.begin();
  strip.setBrightness(100);
}




void loop() {
  // put your main code here, to run repeatedly:
  // RPM input signal - Map the raw input signal to however 
  int RPM_Signal = map(analogRead(0), 0, 1023, 0, 16);
  
  if ((prev_RPM_Signal != RPM_Signal) or (RPM_Signal == 16)) {
    prev_RPM_Signal = RPM_Signal;
    updateLEDs(RPM_Signal);
  }
  
}

void updateLEDs(int RPM) {
  // Updates LEDs to indicate RPM 
  // Tachometer increases/decreases linearly
  if (0 <= RPM and RPM < 16) {
      // Below max RPM
      for (int i = 0; i < LED_len; i++) {
        if (i <= RPM) {
          strip.setPixelColor(i, ledArray[i]);
        }
        else {
          strip.setPixelColor(i, 0);
        }
      }
      strip.show();
    }
    else if (RPM == 16) {
    // At max RPM - Flash red
    // Red on
    for (int i = 0; i < LED_len; i++) {
      strip.setPixelColor(i, red_max);
      }
    strip.show();
    delay(100);
    
    // Red off
    strip.clear();
    strip.show();
    delay(100);
    }
    else {
      // Error indicator - RPM signal is out of range
      strip.clear();
      for (int i = 0; i < LED_len; i = i + 2) {
        strip.setPixelColor(i, blue);
      }
      strip.show();
    }
}

