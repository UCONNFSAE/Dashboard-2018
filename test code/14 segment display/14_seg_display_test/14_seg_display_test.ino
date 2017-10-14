
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

String s = "   WELCOME MATT";

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

void setup()
{
  alpha4.begin(0x70);
  alpha4.setBrightness(1);
}

void loop()
{
    for(int i = 0; i <= s.length(); i++)
    {
      for(int j = 0; j < 4; j++)
      {
        alpha4.writeDigitAscii(j,s[i+j]);
        if(i+j >= s.length())
        {
          alpha4.writeDigitAscii(j, ' ');
        }
      }
      
    delay(200);
    alpha4.writeDisplay();
    }
    for(int i = s.length(); i >= 0; i--)
  {
    alpha4.writeDigitAscii(3,s[i+3]);
    alpha4.writeDigitAscii(2,s[i+2]);
    alpha4.writeDigitAscii(1,s[i+1]);
    alpha4.writeDigitAscii(0,s[i]);

    if(i+3 >= s.length()) alpha4.writeDigitAscii(3,' ');
    if(i+2 >= s.length()) alpha4.writeDigitAscii(2,' ');
    if(i+1 >= s.length()) alpha4.writeDigitAscii(1,' ');
    if(i >= s.length()) alpha4.writeDigitAscii(0,' ');
    
    delay(200);
    alpha4.writeDisplay();
  }

}
