/*
   Version Notes:
    Beta: Only reads engine RPM from ECU and prints to serial monitor.
    v1.0: Added support for Adafruit NeoPixel as tachometer/shift LEDs.
    v2.0: Added support for Adafruit Alphanumeric Backpacks.
    v3.0: Added support for Gear Position from ECU and prints to Alphanumeric Backpack.  Fixed brackets and comments for typos and clarity.
    v4.0: Added support for Wheel Speed and Coolant Temperature from ECU.  Can print to Alphanumeric Backpack.  Build for unveiling.
    v5.0: Added support for Oil Pressure from ECU.  Can print to Alphanumeric Backpack.
    v5.1: Bug fixes.
    v6.0: Removed various functions such as 14-segment displays. Added support for a 7-segment display and PWM driver.
*/

// Libraries:
#include <mcp_can.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_TLC59711.h>

// LED NeoPixel strip parameters
#define SPI_CS_PIN 9                                  // Declares D9 as CS for Seeed's CAN-BUS Shield.
#define stripLength 16                                // NeoPixel LED strip length of 16
#define stripDataPin 5                                // Declares D5 for strip data.
#define blinkInterval 50                              // flashing blink interval in ms

// Segment display paramaters
#define segmentLength 7                               // Segment display length of 7
#define segmentDataPin 3                              // D3 for segment display data

// Absolute brightness control for LEDs
#define stripBrightness 200                                // 0 = off, 255 = fullbright
#define segBrightness 255                                  // brightness settings here set absolute values

// TLC59711 parameters
#define NUM_TLC59711 1 // amount of chips connected
#define data 8         // PWM data pin (any digital pin)
#define clock 7        // PWM clock pin (any digital pin)
#define ENG_LED 0      // engine LED R0
#define OIL_LED 1      // EOP LED G0
int DUTY = 5;          // values from 0 to 100% representing duty cycle

// Engine parameters                                         CHANGE VALUES HERE
#define low_rpm 2000
#define high_rpm 10000
#define HIGH_ENG_TEMP 100
#define UNSAFE_ENG_TEMP 105
#define HIGH_EOP 55

// Library Initializations:
MCP_CAN CAN(SPI_CS_PIN);      // Sets CS pin.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(stripLength, stripDataPin, NEO_GRB + NEO_KHZ800);      // Configures the NeoPixel Strip for 16 LEDs.
Adafruit_NeoPixel seg = Adafruit_NeoPixel(segmentLength, segmentDataPin, NEO_GRB + NEO_KHZ800);     // Configures 7-segment display for 8(actually 7) segments

uint16_t PWM_LEVEL = map(DUTY, 0, 100, 0, 65535); // (DUTY/100) * 65535; PWM value ranges from 0 to 65535
Adafruit_TLC59711 LED = Adafruit_TLC59711(NUM_TLC59711, clock, data); // initialization with parameters (chip amount, clock pin, data pin)

// Define LED colors, these are relative to the absolute brightness set above; MAY CHANGE BRIGHTNESS
uint32_t greenStripDay = strip.Color(0, 50, 0),       // daytime brightness
         yellowStripDay = strip.Color(50, 50, 0),
         redStripDay = strip.Color(50, 0, 0),
         redMaxStripDay = strip.Color(150, 0, 0),
         
         greenStripNight = strip.Color(0, 10, 0),     // lower brightness for evening
         yellowStripNight = strip.Color(10, 10, 0),   
         redStripNight = strip.Color(10, 0, 0),
         redMaxStripNight = strip.Color(30, 0, 0),
         color[4] = {greenStripDay, yellowStripDay, redStripDay, redMaxStripDay};


// 7 segment display colors
uint32_t redSegDay = seg.Color(255, 0, 0),               // daytime brightness
         yellowSegDay = seg.Color(255, 255, 0),
         greenSegDay = seg.Color(0, 255, 0),
         blueSegDay = seg.Color(0, 0, 255),
         whiteSegDay = seg.Color(255, 255, 255),

         redSegNight = seg.Color(30, 0, 0),              // evening brightness
         yellowSegNight = seg.Color(30, 30, 0),
         greenSegNight = seg.Color(0, 30, 0),
         whiteSegNight = seg.Color(30, 30, 30),

         redSeg = redSegDay,                             // initial display brightness
         yellowSeg = yellowSegDay,
         greenSeg = greenSegDay,
         segColor = greenSeg;

// 7 segment display gear
int digitArray[7][7] = {                    //                            ___________      
                    {0, 0, 1, 0, 1, 0, 1},  // n                         |     0     |
                    {0, 1, 1, 0, 0, 0, 0},  // 1                         |  5     1  |
                    {1, 1, 0, 1, 1, 0, 1},  // 2                         |     6     |
                    {1, 1, 1, 1, 0, 0, 1},  // 3                         |  4     2  |
                    {0, 1, 1, 0, 0, 1, 1},  // 4                         |     3     |
                    {1, 0, 1, 1, 0, 1, 1},  // 5                         |___________|
                    {1, 0, 1, 1, 1, 1, 1}   // 6                         
                    };

int digitCircle[6][7] = {
                    {1, 0, 0, 0, 0, 0, 0},
                    {0, 1, 0, 0, 0, 0, 0},
                    {0, 0, 1, 0, 0, 0, 0},
                    {0, 0, 0, 1, 0, 0, 0},
                    {0, 0, 0, 0, 1, 0, 0},
                    {0, 0, 0, 0, 0, 1, 0}
                    };

int shiftPT[2] = {low_rpm, high_rpm};   // sets lower and upper limits of RPMs to display

int prev_range = 10,        // needed for tachometer updating, arbitrary number
    prev_gear = 10,         // needed for gear position updating, arbitrary number
    ledStages[2] = {5, 11}, // this is where each stage of the led strip is set. i.e. from ledStages[0] and ledStages[1] is stage one and so on
    led_pos = 0;            // position of LEDs in sleep mode
long prevBlinkTime = 0;     // timer for warning flash interval

bool warningState = false,  // blink state for shift point flashing
     eng_state = false,     // blink state for engine warning LED
     eop_state = false;     // blink state for oil pressure warning LED

void setup()
{
  //Serial.begin(115200);     // comment out all Serials and Strings when not testing
  do
  {
    if (CAN_OK == CAN.begin(CAN_1000KBPS))  // initializes CAN-BUS Shield at specified baud rate.
    {
      //Serial.println("CAN-BUS Shield Initialized!");
      //is_CBS_init = true;
      break;
    }
    else
    {
      //Serial.println("CAN-BUS Shield FAILED!");
      //Serial.println("Retry initializing CAN-BUS Shield.");
      delay(1000);
    }
  }
  while(true);
  
  // begin NeoPixel strip
  strip.begin();
  strip.setBrightness(stripBrightness);
  strip.clear();
  strip.show();

  // begin segment display
  seg.begin();
  seg.setBrightness(segBrightness);
  seg.clear();
  seg.show();

  LED.begin();
  
  startupAnimation();

  color[0] = greenStripDay;
  color[1] = yellowStripDay;
  color[2] = redStripDay;
  color[3] = redMaxStripDay;

  segColor = whiteSegDay;
}

void loop()
{
  unsigned char len = 0;
  unsigned char buf[8];
  if(CAN_MSGAVAIL == CAN.checkReceive()) //Checks for incoming data.
  {
    CAN.readMsgBuf(&len, buf);           //Reads incoming data. len: data length, buf[location]: actual data.
    unsigned int canId = CAN.getCanId();
    //String id = String(canId);
    //Serial.println(id);
    
    if (canId == 1536)
    {
      int ENGINE_RPM = 0;
      int rpmA = buf[0];
      int rpmB = buf[1];
      ENGINE_RPM = ((rpmA * 256) + rpmB);
      ledStrip_update(ENGINE_RPM);
      //gearOFF(ENGINE_RPM);
      //String ALPHA_RPM = String(ENGINE_RPM);
      //Serial.println("RPM: " + ALPHA_RPM + "\n");
    }

    if (canId == 1550)
    {
      int gearA = buf[2];
      int gearB = buf[3];
      int GEAR = ((gearA * 256) + gearB - 2);
      gearShift_update(GEAR);
      //String ALPHA_GEAR = String(GEAR);
      //Serial.println("GEAR: " + ALPHA_GEAR + "\n");
    }
  
    if (canId == 1541)
    {
      int tempA = buf[2];
      int tempB = buf[3];
      int ENGINE_TEMP = ((tempA * 256) + tempB) / 10;
      TEMP_warning(ENGINE_TEMP);
      //String ALPHA_TEMP = String(ENGINE_TEMP);
      //Serial.println("ENG TEMP: " + ALPHA_TEMP + "\n");
    }

    if (canId == 1544)
    {
      int eopA = buf[0];
      int eopB = buf[1];
      int INT_EOP = (((eopA * 256) + eopB) * 0.0145037738); // millibars to PSI conversion
      EOP_warning(INT_EOP);
      //String ALPHA_EOP = String(INT_EOP);
      //Serial.println("EOP: " + ALPHA_EOP + "\n");
    }
  }
}

void ledStrip_update(int rpm)   // tachometer function
{
  int LED_RPM = rpm;
  unsigned long currentMillis = millis();   // get current timer for flashing condition
  if (LED_RPM >= shiftPT[0] && LED_RPM < shiftPT[1]) // if the RPM is between the lowest RPM and the shift point
  {
    // map the RPM values from 0 to 16(really 15 since the shift point and beyond is handled below) and constrain the range
    int rpmMapped = map(LED_RPM, shiftPT[0], shiftPT[1], 0, 16);
    int rpmConstrained = constrain(rpmMapped, 0, 16);

    if (prev_range != rpmConstrained)                // this makes it so we only update the LED when the range changes so we don't readdress the strip every reading
    {
      prev_range = rpmConstrained;
      strip.clear();
      for (int ledNum = 0; ledNum <= rpmConstrained; ledNum++) // rpmConstrained makes ledNum refer to one LED too much on the light strip since the light strip is indexed from 0,
                                                               // causes brief moment of max rpm before flashing to change gear
      {
        if (ledNum <= ledStages[0])                            // green
        {
          strip.setPixelColor(ledNum, color[0]);
        }
        else if (ledNum > ledStages[0] && ledNum <= ledStages[1])   // yellow
        {
          strip.setPixelColor(ledNum, color[1]);
        }
        else if (ledNum > ledStages[1] && ledNum < stripLength)     // red
        {
          strip.setPixelColor(ledNum, color[2]);
        }
      }
      strip.show();
    }
  }
  else if (LED_RPM >= shiftPT[1]) // RPM at or above shift point, blinks the LEDS on and off with no blocking delay
  {
    prev_range = 16;
    if (currentMillis - prevBlinkTime > blinkInterval)    // millis eventually overflows but only after 50 days powered on so abs value is not needed here
    {
      prevBlinkTime = currentMillis;

      if (warningState == false)
      {
        warningState = true;
        for (int i = 0; i < stripLength; i++)
        {
          strip.setPixelColor(i, color[3]);
        }
        strip.show();
      } 
      else
      {
        strip.clear();
        strip.show();
        warningState = false;
      }
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

void gearShift_update(int gear) // update and display gear number on segment display
{
    seg.clear();
    for(int i = 0; i <= 6; i++)
    {
      if(digitArray[gear][i])
        seg.setPixelColor(i, whiteSegDay);
    }
    seg.show();
}

void TEMP_warning(int engine_temp) // turn LED on when engine temp is outside safe parameters
{
  if (engine_temp >= HIGH_ENG_TEMP && engine_temp < UNSAFE_ENG_TEMP)
  {
    LED.setPWM(ENG_LED, PWM_LEVEL);
  }
  else if (engine_temp >= UNSAFE_ENG_TEMP)
  {
    if (eng_state == false)
    {
      LED.setPWM(ENG_LED, PWM_LEVEL);
      eng_state = true;
    }
    else
    {
      LED.setPWM(ENG_LED, 0);
      eng_state = false;
    }
  }
  else
  {
    LED.setPWM(ENG_LED, 0);
  }
  LED.write();
}

void EOP_warning(int EOP) // turn LED on when oil pressure is outside safe parameters
{
  if (EOP >= HIGH_EOP)
  {
    LED.setPWM(OIL_LED, PWM_LEVEL);
  }
  else if (EOP < 8)
  {
    if (eop_state == false)
    {
      LED.setPWM(OIL_LED, PWM_LEVEL);
      eop_state = true;
    }
    else
    {
      LED.setPWM(OIL_LED, 0);
      eop_state = false;
    }
  }
  else
  {
    LED.setPWM(OIL_LED, 0);
  }
  LED.write();
}

void startupAnimation()
{
  LED.setPWM(ENG_LED, PWM_LEVEL);
  LED.setPWM(OIL_LED, PWM_LEVEL);
  LED.write();
  segColor = greenSeg;
  for(int h = 0; h <= 15; h++)
  {
    for(int i = 0; i <= 5; i++)
    {
      for(int j = 0; j <= 6; j++)
      {
        if(digitCircle[i][j])
          seg.setPixelColor(j, segColor);
      }
      seg.show();
      delay(30);
      seg.clear();
    }

    if (h <= ledStages[0])
    {
      strip.setPixelColor(h, color[0]);
      segColor = greenSeg;
    }
    else if (h > ledStages[0] && h <= ledStages[1])
    {
      strip.setPixelColor(h, color[1]);
      segColor = yellowSeg;
    }
    else if (h > ledStages[1] && h < stripLength)
    {
      strip.setPixelColor(h, color[2]);
      segColor = redSeg;
    }
    strip.show();
  }
  seg.clear();
  seg.show();
  LED.setPWM(ENG_LED, 0);
  LED.setPWM(OIL_LED, 0);
  LED.write();
  delay(500);
  strip.clear();
  strip.show();
  delay(1000);
}

