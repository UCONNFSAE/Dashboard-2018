
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

String s = "   WELCOME MATT";

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

void setup()
{
  alpha4.begin(0x70);
  alpha4.setBrightness(1);

  for(int i = 0; i <= s.length(); i++)
  {
    alpha4.writeDigitAscii(3,s[i+3]);
    alpha4.writeDigitAscii(2,s[i+2]);
    alpha4.writeDigitAscii(1,s[i+1]);
    alpha4.writeDigitAscii(0,s[i]);

    if(i+3 >= s.length()) alpha4.writeDigitAscii(3,' ');
    if(i+2 >= s.length()) alpha4.writeDigitAscii(2,' ');
    if(i+1 >= s.length()) alpha4.writeDigitAscii(1,' ');
    if(i >= s.length()) alpha4.writeDigitAscii(0,' ');
    
    delay(400);
    alpha4.writeDisplay();
  }
}

void loop()
{
  

}
