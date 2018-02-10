#include <Adafruit_TLC59711.h>
#include <SPI.h>
#include <TimerOne.h>

#define NUM_TLC59711 1 //amount of chips connected
#define data 11 //SPI data pin on arduino
#define clock 13 //SPI clock pin on arduino
#define OIL_PR 0
#define ENG_TMP 1
#define OFF 0
#define ON 1

uint8_t DUTY = 45; // Values range from 1 to 100% to determine duty cycle
uint16_t PWM_LEVEL = map(DUTY, 0, 100, 0, 65535);//(DUTY/100) * 65535; // PWM value ranges from 0 to 65535
volatile int next_state = 0;

Adafruit_TLC59711 LED = Adafruit_TLC59711(NUM_TLC59711, clock, data); // new object with parameters (chip amount, clock pin, data pin) 

void setup() {
  LED.begin(); 
  LED.write(); 
  Serial.begin(9600); // baud rate
  Serial.print(PWM_LEVEL);
  Timer1.initialize(200000); //counts timer in microseconds
  Timer1.attachInterrupt(blink_LED);
}

void loop() {
  /*
  LED.setPWM(OIL_PR, PWM_LEVEL);
  LED.write();
  delay(100);
  LED.setPWM(OIL_PR, 0);
  LED.write();
  delay(100);
  */
}

void blink_LED()
{
  if (next_state == OFF)
  {
    LED.setPWM(OIL_PR, 0);
    LED.write();
    next_state = ON;
  }
  else
  {
    LED.setPWM(OIL_PR, PWM_LEVEL);
    LED.write();
    next_state = OFF;
  }
}


