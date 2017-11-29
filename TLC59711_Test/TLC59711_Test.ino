#include <Adafruit_TLC59711.h>
#include <SPI.h>

#define NUM_TLC59711 1
#define data 11
#define clock 13
#define OIL_PR 0
#define ENG_TMP 1

uint8_t DUTY = 45; // Values range from 1 to 100% to determine duty cycle
uint16_t PWM_LEVEL = map(DUTY, 0, 100, 0, 65535);//(DUTY/100) * 65535; // PWM value ranges from 0 to 65535

Adafruit_TLC59711 LED = Adafruit_TLC59711(NUM_TLC59711, clock, data);
 
void setup() {
  LED.begin();
  LED.write();
  Serial.begin(9600);
  Serial.print(PWM_LEVEL);
}

void loop() {
  LED.setPWM(OIL_PR, PWM_LEVEL);
  LED.write();
  delay(100);
  LED.setPWM(OIL_PR, 0);
  LED.write();
  delay(100);

}
