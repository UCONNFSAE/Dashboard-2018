#include <Adafruit_TLC59711.h>
//#include <SPI.h>

#define numTLC59711 1
#define dataPin 13
#define clkPin 12

#define On (65535 +1)/ 2/2/2

#define rgbReadPin 7
#define ledReadPin 6




Adafruit_TLC59711 pwm = Adafruit_TLC59711(numTLC59711, clkPin, dataPin);

uint8_t ledPot = analogRead(ledReadPin);
uint8_t rgb;

int digits[7][7] = {{0, 0, On, 0, On, 0, On},     // n 
                    {0, On, On, 0, 0, 0, 0},      // 1
                    {On, On, 0, On, On, 0, On},   // 2
                    {On, On, On, On, 0, 0, On},   // 3
                    {0, On, On, 0, 0, On, On},    // 4
                    {On, 0, On, On, 0, On, On},   // 5
                    {On, 0, On, On, On,  On, On}  // 6
                    };


void setup() {
  pwm.begin();
  pwm.write();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i <= 6; i++) {
      pwm.setPWM(i, digits[0][i]);
  }
  pwm.write();

  delay(500);

  for (int i = 0; i <= 6; i++) {
      pwm.setPWM(i, digits[1][i]);
  }
  pwm.write();
  delay(500);

  for (int i = 0; i <= 6; i++) {
      pwm.setPWM(i, digits[2][i]);
  }
  pwm.write();
  delay(500);

  for (int i = 0; i <= 6; i++) {
      pwm.setPWM(i, digits[3][i]);
  }
  pwm.write();
  delay(500);

  for (int i = 0; i <= 6; i++) {
      pwm.setPWM(i, digits[4][i]);
  }
  pwm.write();
  delay(500);

  for (int i = 0; i <= 6; i++) {
      pwm.setPWM(i, digits[5][i]);
  }
  pwm.write();
  delay(500);

  for (int i = 0; i <= 6; i++) {
      pwm.setPWM(i, digits[6][i]);
  }
  pwm.write();
  delay(500);

  pwm.setPWM(11, 5000);
  pwm.write();
}


void pwmClear() {

}


