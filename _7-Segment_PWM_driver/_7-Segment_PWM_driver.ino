#include <Adafruit_TLC59711.h>
#include <SPI.h>

#define NUM_TLC59711 1
#define data 11
#define clock 13
#define OIL_PR 0
#define ENG_TMP 1

#define SegA 7
#define SegB 6
#define SegC 2
#define SegD 1
#define SegE 0
#define SegF 9
#define SegG 8


uint8_t Duty = 45; // Values range from 1 to 100% to determine duty cycle
uint16_t PWM_Level = map(Duty, 0, 100, 0, 65535);//(DUTY/100) * 65535; // PWM value ranges from 0 to 65535

Adafruit_TLC59711 LED = Adafruit_TLC59711(NUM_TLC59711, clock, data);

void setup() {
  // put your setup code here, to run once:
  LED.begin();
  LED.write();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  // Display n
  LED.setPWM(SegE, PWM_Level);
  LED.setPWM(SegG, PWM_Level);
  LED.setPWM(SegC, PWM_Level);

  // Display 1
  LED.setPWM(SegB, PWM_Level);
  LED.setPWM(SegC, PWM_Level);
  
  // Display 2
  LED.setPWM(SegA, PWM_Level);
  LED.setPWM(SegB, PWM_Level);
  LED.setPWM(SegD, PWM_Level);
  LED.setPWM(SegE, PWM_Level);
  LED.setPWM(SegG, PWM_Level);
  
  // Display 3
  LED.setPWM(SegA, PWM_Level);
  LED.setPWM(SegB, PWM_Level);
  LED.setPWM(SegC, PWM_Level);
  LED.setPWM(SegD, PWM_Level);
  LED.setPWM(SegG, PWM_Level);
  
  // Display 4
  LED.setPWM(SegB, PWM_Level);
  LED.setPWM(SegC, PWM_Level);
  LED.setPWM(SegF, PWM_Level);
  LED.setPWM(SegG, PWM_Level);

  // Display 5
  LED.setPWM(SegA, PWM_Level);
  LED.setPWM(SegC, PWM_Level);
  LED.setPWM(SegD, PWM_Level);
  LED.setPWM(SegF, PWM_Level);
  LED.setPWM(SegG, PWM_Level);
  
  // Display 6
  LED.setPWM(SegA, PWM_Level);
  LED.setPWM(SegC, PWM_Level);
  LED.setPWM(SegD, PWM_Level);
  LED.setPWM(SegE, PWM_Level);
  LED.setPWM(SegF, PWM_Level);
  LED.setPWM(SegG, PWM_Level);
  
  


  
}
