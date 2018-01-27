/*
   Version Notes:
    Beta: Only reads engine RPM from ECU and prints to serial monitor.
    v1.0: Added support for Adafruit NeoPixel as tachometer/shift LEDs.
    v2.0: Added support for Adafruit Alphanumeric Backpacks.
    v3.0: Added support for Gear Position from ECU and prints to Alphanumeric Backpack.  Fixed brackets and comments for typos and clarity.
    v4.0: Added support for Wheel Speed and Coolant Temperature from ECU.  Can print to Alphanumeric Backpack.  Build for unveiling.
    v5.0: Added support for Oil Pressure from ECU.  Can print to Alphanumeric Backpack.
    v5.1: Bug fixes.
*/

// Libraries:
#include <mcp_can.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_LEDBackpack.h>
#include <Wire.h>
#include <Adafruit_TLC59711.h>
#include <TimerOne.h>


//led neopixel strip
#define SPI_CS_PIN 9                                   // Declares D9 as CS for Seeed's CAN-BUS Shield.
#define PIN 6                                          // Declares D6 for NeoPixel data.
#define led_len 6


//TLC59711
#define NUM_TLC59711 1 //amount of chips connected
#define data 2 //PWM data pin (any digital pin)
#define clock 3 //PWM clock pin (any digital pin)
#define ENG_LED 0
#define OIL_LED 1
#define WARN_ENG_TEMP 200
#define WARN_EOP 200


//Library Initialization:
MCP_CAN CAN(SPI_CS_PIN);                                                        // Sets CS pin.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);      // Configures the NeoPixel Strip for 16 LEDs.


//Define LED colors
uint32_t green = strip.Color(0, 20, 0),
         yellow = strip.Color(20, 20, 0),
         red = strip.Color(20, 0, 0),
         red_max = strip.Color(100, 0, 0),
         blue = strip.Color(0, 0, 20),
         color[4] = {green, yellow, red, red_max};


//TLC59711 parameters and initialization
uint8_t DUTY = 45; // Values range from 1 to 100% to determine duty cycle
uint16_t PWM_LEVEL = map(DUTY, 0, 100, 0, 65535); //(DUTY/100) * 65535; // PWM value ranges from 0 to 65535
volatile int next_state = 0;
Adafruit_TLC59711 LED = Adafruit_TLC59711(NUM_TLC59711, clock, data); // new object with parameters (chip amount, clock pin, data pin)


//CAN parameters
bool is_CBS_init = false; //True = CAN-Bus initialized succesfully. False = Not initialized yet.
//char buffer[512];  //Data will be temporarily stored to this buffer before being written to the file

float LED_RPM;
int shiftPT[2] = {2000, 10000}; //PLACEDHOLDER VALUES, TBD LATER

int prev_range = 10,
    ledStages[2] = {5, 11}, // this is where each stage of the led strip is set. i.e. from ledStages[0] and ledStages[1] is stage one and so on
    warningState = 0,
    blinkInterval = 150,
    stripBrightness = 100;      // 0 = off, 255 = fullbright
long prevBlinkTime = 0;

int ENGINE_TEMP = 0,
    INT_EOP = 0;

void setup()
{
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(stripBrightness);
  strip.show();

  Timer1.initialize(100000); //counts timer in microseconds
  Timer1.attachInterrupt(WARNING_LED);

  do //while(!is_CBS_init)
  {
    if (CAN_OK == CAN.begin(CAN_1000KBPS))  //Initializes CAN-BUS Shield at specified baud rate.
    {
      Serial.println("CAN-BUS Shield Initialized!");
      //blink_led(2, 150, color[0]); //WHY is this blinking?
      is_CBS_init = true;
    }
    else
    {
      Serial.println("CAN-BUS Shield FAILED!");
      Serial.println("Retry initializing CAN-BUS Shield.");
      //blink_led(3, 500, color[2]);
      delay(1000);
    }
  }
  while(!is_CBS_init)
}

void loop()
{
  unsigned char len = 0;
  unsigned char buf[8];

  if(CAN_MSGAVAIL == CAN.checkReceive()) //Checks for incoming data.
  {
    CAN.readMsgBuf(&len, buf);           //Reads incoming data. len: data length, buf[location]: actual data.
    unsigned long canId = CAN.getCanId();
  
    if (canId == 1536)
    {
      int rpmA = buf[0];
      int rpmB = buf[1];
      int ENGINE_RPM = ((rpmA * 256) + rpmB);
      String ALPHA_RPM = String(ENGINE_RPM / 10);
      ledStrip_update(ENGINE_RPM);
      Serial.println(ALPHA_RPM);
    }

    if (canId == 1550)
    {
      int gearA = buf[2];
      int gearB = buf[3];
      String ALPHA_GEAR = String((gearA * 256) + gearB - 2);
      Serial.println(ALPHA_GEAR);
    }
  
    if (canId == 1542)
    {
      int tempA = buf[2];
      int tempB = buf[3];
      ENGINE_TEMP = ((tempA * 256) + tempB) / 10;
      String ALPHA_TEMP = String(ENGINE_TEMP);
      Serial.println(ALPHA_TEMP);
    }

    if (canId == 1544)
    {
      int eopA = buf[0];
      int eopB = buf[1];
      INT_EOP = ((((eopA * 256) + eopB) - 921) * 0.0145037738);
      String ALPHA_EOP = String((((eopA * 256) + eopB) - 921) * 0.0145037738);
      Serial.println(ALPHA_EOP);
    }
  }
}

void ledStrip_update(uint16_t LED_RPM) //MODIFY TO CURRENT!!!
{
  unsigned long currentMillis = millis();
  if (LED_RPM >= shiftPT[0] && LED_RPM < shiftPT[1]) //if the RPM is between the activation pt and the shift pt
  {
    //map the RPM values to 9(really 8 since the shift point and beyond is handled below) and constrain the range
    int rpmMapped = map(LED_RPM, shiftPT[0], shiftPT[1], 0, 16);
    int rpmConstrained = constrain(rpmMapped, 0, 16);

    if (prev_range != rpmConstrained) //This makes it so we only update the LED when the range changes so we don't readdress the strip every reading
    {
      prev_range = rpmConstrained;
      strip.clear();
      strip.show();
      for (int ledNum = 0; ledNum <= rpmConstrained; ledNum++)
      {
        if (ledNum <= ledStages[0])
        {
          strip.setPixelColor(ledNum, color[0]);
        }
        else if (ledNum > ledStages[0] && ledNum <= ledStages[1])
        {
          strip.setPixelColor(ledNum, color[1]);
        }
        else if (ledNum > ledStages[1] && ledNum < strip.numPixels())
        {
          strip.setPixelColor(ledNum, color[2]);
        }
      }
      strip.show();
    }
  }
  else if (LED_RPM >= shiftPT[1]) //SHIFT DAMNIT!! This blinks the LEDS back and forth with no delay to block button presses
  {
    prev_range = 16;
    if (currentMillis - prevBlinkTime > blinkInterval)
    {
      prevBlinkTime = currentMillis;

      if (warningState == 0)
      {
        warningState = 1;
        for (int i = 0; i < strip.numPixels(); i++)
        {
          strip.setPixelColor(i, color[3]);
        }
      } 
      else
      {
        strip.clear();
      }
      strip.show();
    }
  }
  else
  {
    if (prev_range != 10)
    {
      prev_range = 10;
      strip.clear(); 
      strip.show();
    }
  }
}

void WARNING_LED() //Turn LED on when engine temp or oil pressure are outside safe parameters
{
  if(ENGINE_TEMP > WARN_ENG_TEMP)
  {
    LED.setPWM(ENG_LED, PWM_LEVEL);
    LED.write();
  }
  else
  {
    LED.setPWM(ENG_LED, 0);
    LED.write();
  }

  if(INT_EOP > WARN_EOP)
  {
    LED.setPWM(OIL_LED, PWM_LEVEL);
    LED.write();
  }
  else
  {
    LED.setPWM(OIL_LED, 0);
    LED.write();
  }
}

void blink_led(int count, int ms_delay, int colorInt)
{
  strip.clear();
  strip.show();
  for(int i = 0; i < count; i++)
  {
    strip.setPixelColor(0, colorInt);
    strip.show();
    delay(ms_delay);
    strip.clear();
    strip.show();
    delay(ms_delay/2);
  }
  strip.clear();
  strip.show();
}





