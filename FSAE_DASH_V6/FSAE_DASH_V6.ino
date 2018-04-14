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

// LED neopixel strip parameters
#define SPI_CS_PIN 9                                  // Declares D9 as CS for Seeed's CAN-BUS Shield.
#define stripLength 16                                // NeoPixel LED strip length of 16
#define stripDataPin 5                                // Declares D5 for strip data.

// Segment display paramaters
#define segmentLength 8                               // Segment display "length" of 8
#define segmentDataPin 3                              // D3 for segment display data

#define brightness 100                           // 0 = off, 255 = fullbright

// TLC59711 parameters
#define NUM_TLC59711 1 // amount of chips connected
#define data 8         // PWM data pin (any digital pin)
#define clock 7        // PWM clock pin (any digital pin)
#define ENG_LED 0      // engine LED R0
#define OIL_LED 1      // EOP LED G0
#define DUTY 10        // values from 0 to 100% representing duty cycle

// Engine parameters                                         CHANGE VALUES HERE
#define low_rpm 2200
#define high_rpm 10000
#define WARN_ENG_TEMP 90
#define WARN_EOP 55

// Library Initializations:
MCP_CAN CAN(SPI_CS_PIN);      // Sets CS pin.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(stripLength, stripDataPin, NEO_GRB + NEO_KHZ800);      // Configures the NeoPixel Strip for 16 LEDs.
Adafruit_NeoPixel seg = Adafruit_NeoPixel(segmentLength, segmentDataPin, NEO_GRB + NEO_KHZ800);     // Configures 7-segment display for 8(actually 7) segments

uint16_t PWM_LEVEL = map(DUTY, 0, 100, 0, 65535); // (DUTY/100) * 65535; PWM value ranges from 0 to 65535
Adafruit_TLC59711 LED = Adafruit_TLC59711(NUM_TLC59711, clock, data); // initialization with parameters (chip amount, clock pin, data pin)

// Define LED colors, MAY CHANGE BRIGHTNESS
uint32_t greenStrip = strip.Color(0, 20, 0),
         yellowStrip = strip.Color(20, 20, 0),
         redStrip = strip.Color(20, 0, 0),
         redMaxStrip = strip.Color(100, 0, 0),
         blueStrip = strip.Color(0, 0, 20),
         color[4] = {greenStrip, yellowStrip, redStrip, redMaxStrip};

// 7 segment display gear
int digitArray[7][8] = {                       //                            ___________                ___________      
                    {0, 0, 1, 0, 1, 0, 1, 0},  // n                         |     0     |              |     a     |
                    {0, 1, 1, 0, 0, 0, 0, 0},  // 1                         |  5     1  |              |  f     b  |
                    {1, 1, 0, 1, 1, 0, 1, 0},  // 2                         |     6     |              |     g     |
                    {1, 1, 1, 1, 0, 0, 1, 0},  // 3                         |  4     2  |              |  e     c  |
                    {0, 1, 1, 0, 0, 1, 1, 0},  // 4                         |     3     |              |     d     |
                    {1, 0, 1, 1, 0, 1, 1, 0},  // 5                         |_________7_|              |_________h_|
                    {1, 0, 1, 1, 1, 1, 1, 0}   // 6                         
                    };

// 7 segment display startup animation
int digitHello[5][8] = {
                    {0, 1, 1, 0, 1, 1, 1, 0}, //H
                    {1, 0, 0, 1, 1, 1, 1, 0}, //E
                    {0, 0, 0, 1, 1, 1, 0, 0}, //L
                    {0, 0, 0, 1, 1, 1, 0, 0}, //L
                    {1, 1, 1, 1, 1, 1, 0, 0}  //O
                    };

// 7 segment display colors
uint32_t redSeg = seg.Color(255, 0, 0),
         orangeSeg = seg.Color(255, 165, 0),
         yellowSeg = seg.Color(255, 255, 0),
         greenSeg = seg.Color(0, 255, 0),
         blueSeg = seg.Color(0, 0, 255),
         purpleSeg = seg.Color(255, 0, 255),
         whiteSeg = seg.Color(255, 255, 255),
         segColor = greenSeg,
         prev_segColor;

//float LED_RPM;
int shiftPT[2] = {low_rpm, high_rpm};

int prev_range = 10,        // needed for tachometer updating
    prev_gear = 10,         // needed for gear position updating
    ledStages[2] = {5, 11}, // this is where each stage of the led strip is set. i.e. from ledStages[0] and ledStages[1] is stage one and so on
    warningState = 0,       // allows warning to oscillate
    blinkInterval = 50;    // warning flash interval
long prevBlinkTime = 0;     // timer for warning flash interval
bool off = false;           // gear indicator off when engine is off

void setup()
{
  Serial.begin(115200);     // comment out all Serials when not testing

  do
  {

    break;          // TEMPORARY - NEEDED FOR TESTING STARTUP ANIMATIONS

    
    if (CAN_OK == CAN.begin(CAN_1000KBPS))  // initializes CAN-BUS Shield at specified baud rate.
    {
      Serial.println("CAN-BUS Shield Initialized!");
      //blink_led(2, 150, color[0]);
      break;
    }
    else
    {
      Serial.println("CAN-BUS Shield FAILED!");
      Serial.println("Retry initializing CAN-BUS Shield.");
      //blink_led(3, 500, color[2]);
      delay(1000);
    }
  }
  while(true);

  // begin NeoPixel strip
  strip.begin();
  strip.setBrightness(brightness);
  strip.clear();
  strip.show();

  // begin 7-segment display
  seg.begin();
  seg.setBrightness(brightness);
  seg.clear();
  seg.show();

  LED.begin();

  startupAnimation();
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
      ledStrip_update(ENGINE_RPM);
      //gearOFF(ENGINE_RPM);
      String ALPHA_RPM = String(ENGINE_RPM);
      Serial.println("RPM: " + ALPHA_RPM + "\n");
    }

    if (canId == 1550)
    {
      int gearA = buf[2];
      int gearB = buf[3];
      int GEAR = ((gearA * 256) + gearB - 2); // why -2? That's how the signal is sent from the ECU through the CAN-bus
      //int GEAR = ((gearA * 256) + gearB);
      gearShift_update(GEAR, segColor);
      String ALPHA_GEAR = String(GEAR);
      Serial.println("GEAR: " + ALPHA_GEAR + "\n");
    }
  
    if (canId == 1542)
    {
      int tempA = buf[2];
      int tempB = buf[3];
      int ENGINE_TEMP = ((tempA * 256) + tempB) / 10;
      TEMP_warning(ENGINE_TEMP);
      String ALPHA_TEMP = String(ENGINE_TEMP);
      Serial.println("ENG TEMP: " + ALPHA_TEMP + "\n");
    }

    if (canId == 1544)
    {
      int eopA = buf[0];
      int eopB = buf[1];
      //int INT_EOP = ((((eopA * 256) + eopB) - 921) * 0.0145037738); // why -921?
      int INT_EOP = (((eopA * 256) + eopB) * 0.0145037738); //millibars to PSI conversion
      EOP_warning(INT_EOP);
      String ALPHA_EOP = String(INT_EOP);
      Serial.println("EOP: " + ALPHA_EOP + "\n");
    }
  }
}

void ledStrip_update(int rpm)
{
  int LED_RPM = rpm;
  unsigned long currentMillis = millis();
  if (LED_RPM >= shiftPT[0] && LED_RPM < shiftPT[1]) // if the RPM is between the activation pt and the shift pt
  {
    // map the RPM values to 9(really 8 since the shift point and beyond is handled below) and constrain the range
    int rpmMapped = map(LED_RPM, shiftPT[0], shiftPT[1], 0, 16);
    int rpmConstrained = constrain(rpmMapped, 0, 16);

    if (prev_range != rpmConstrained)                // this makes it so we only update the LED when the range changes so we don't readdress the strip every reading
    {
      prev_range = rpmConstrained;
      strip.clear();
      for (int ledNum = 0; ledNum <= rpmConstrained; ledNum++) // rpmConstrained makes ledNum refer to one LED too muc on the light strip since the light strip is indexes from 0,
                                                               // causes brief moment of max rpm before flashing to change gear
      {
        if (ledNum <= ledStages[0])
        {
          strip.setPixelColor(ledNum, color[0]);
          segColor = greenSeg;
        }
        else if (ledNum > ledStages[0] && ledNum <= ledStages[1])
        {
          strip.setPixelColor(ledNum, color[1]);
          segColor = yellowSeg;
        }
        else if (ledNum > ledStages[1] && ledNum < strip.numPixels())
        {
          strip.setPixelColor(ledNum, color[2]);
          segColor = redSeg;
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
        strip.show();
      } 
      else
      {
        strip.clear();
        strip.show();
        warningState = 0;
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

void gearOFF(int rpm)
{
  if(rpm == 0)
  {
    seg.clear();
    seg.show();
    off = true;
  }
  else
  {
    off = false;
    prev_segColor = whiteSeg;
  }
}

void gearShift_update(int gear, uint32_t segColor) // update and display gear number on segment display
{
  // short-circuit evaluation optimization
  if ((prev_segColor != segColor || gear != prev_gear) && off == false) {
    prev_segColor = segColor;
    prev_gear = gear;
    seg.clear();
    seg.show();
    for(int i = 0; i <= 7; i++)
    {
      if(digitArray[gear][i])
        seg.setPixelColor(i, segColor);
    }
    seg.show();
  }
}

void TEMP_warning(int engine_temp) // turn LED on when engine temp is outside safe parameters
{
  if(engine_temp >= WARN_ENG_TEMP)
  {
    LED.setPWM(ENG_LED, PWM_LEVEL);
    LED.write();
  }
  else
  {
    LED.setPWM(ENG_LED, 0);
    LED.write();
  }
}

void EOP_warning(int EOP) // turn LED on when oil pressure is outside safe parameters
{
  if(EOP >= WARN_EOP)
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

void startupAnimation()
{
/*  
  for(int i = 0; i <= 4; i++)
  {
    for(int j = 0; j <= 7; j++)
    {
      if(digitHello[i][j])
        seg.setPixelColor(j, blueSeg);
    }
    seg.show();
    delay(500);
    seg.clear();
  }
  seg.clear();
  seg.show();
*/


  uint32_t cycleColor[6] = {whiteSeg, purpleSeg, blueSeg, greenSeg, yellowSeg, redSeg};
/*
  //seg.setPixelColor(6, seg.Color(128, 128, 128));
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 6; j++) {
      seg.setPixelColor(j, cycleColor[i % 3]);
      seg.show();
      delay(25);
    }
  }
  seg.clear();
  seg.show();
*/


  for (int i = 0; i < 16; i++) {
    strip.setPixelColor(i, whiteSeg);
  }
  strip.show();
  

  
  
  for (int i = 0; i < 6; i++) {
    seg.setPixelColor(7, cycleColor[i]);
    seg.show();
    delay(100);
  
    seg.setPixelColor(2, cycleColor[i]);
    seg.setPixelColor(3, cycleColor[i]);
    seg.show();
    delay(100);
  
    seg.setPixelColor(1, cycleColor[i]);
    seg.setPixelColor(4, cycleColor[i]);
    seg.setPixelColor(6, cycleColor[i]);
    seg.show();
    delay(100);
  
    seg.setPixelColor(0, cycleColor[i]);
    seg.setPixelColor(5, cycleColor[i]);
    seg.show();
    delay(100);
  }


/*
  for (int i = 0; i < 5; i++) {
    seg.setPixelColor(0, blueSeg);
    seg.setPixelColor(6, blueSeg);
    seg.show();
    delay(100);

    seg.clear();

    seg.setPixelColor(1, blueSeg);
    seg.setPixelColor(2, blueSeg);
    seg.show();
    delay(100);

    seg.clear();

    seg.setPixelColor(3, blueSeg);
    seg.setPixelColor(6, blueSeg);
    seg.show();
    delay(100);

    seg.clear();

    seg.setPixelColor(4, blueSeg);
    seg.setPixelColor(5, blueSeg);
    seg.show();
    delay(100);

    seg.clear();
  }

  seg.clear();
  seg.show();
*/

/* Color testing
  for (int i = 0; i < 8; i++) {
    seg.setPixelColor(i, Seg);
  }
  seg.show();
  delay(10000);

  seg.clear();
  seg.show();

*/

  seg.clear();
  seg.show();

  strip.clear();
  strip.show();
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
