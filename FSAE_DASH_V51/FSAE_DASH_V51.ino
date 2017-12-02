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
const int SPI_CS_PIN = 9;                                   // Declares D9 as CS for Seeed's CAN-BUS Shield.
#include <Adafruit_NeoPixel.h>
#define PIN 6                                               // Declares D6 for NeoPixel data.
#include <Adafruit_LEDBackpack.h>
#include <Wire.h>



// Library Initialization:
MCP_CAN CAN(SPI_CS_PIN);                                                         // Sets CS pin.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);      // Configures the NeoPixel Strip for 16 LEDs.
Adafruit_AlphaNum4 alpha0 = Adafruit_AlphaNum4(); //leftmost display
Adafruit_AlphaNum4 alpha1 = Adafruit_AlphaNum4(); //middle display
Adafruit_AlphaNum4 alpha2 = Adafruit_AlphaNum4(); //right most display;



// Variables:
uint32_t
green = strip.Color(0, 255, 0),
yellow = strip.Color(255, 255, 0),
red = strip.Color(255, 0, 0),
color[3] = {green, yellow, red};

bool is_CBS_init = false; //True = CAN-Bus initialized succesfully. False = Not initialized yet.

char buffer[512];  //Data will be temporarily stored to this buffer before being written to the file
long prevBlinkTime = 0;

int
prev_range = 10,
shiftPT[2] = {2000, 10000},  // point 0 = the activation point 1 = warning point
ledStages[8] = {0, 3, 5, 7, 9, 11, 13, 15}, // this is where each stage of the led strip is set. i.e. from ledStages[0] and ledStages[1] is stage one and so on
warningState = 0,
blinkInterval = 150,  
stripBrightness = 180;      // 0 = off, 255 = fullbright

float LED_RPM;



// Setup Loop:
void setup()
{
  //Main setup loop.
  Serial.begin(115200);                                   // Sets Serial Monitor baud rate.
  strip.begin(); //initialize the LED Strip.
  strip.setBrightness(stripBrightness);
  strip.show(); // Initialize all pixels to 'off'
  alpha0.begin(0x70);  //No Pins Shorted
  alpha1.begin(0x71);  //A0 Shorted = +1
  alpha2.begin(0x72);  //A1 Shorted = +2


  while (!is_CBS_init)
  {
    if (CAN_OK == CAN.begin(CAN_1000KBPS)) {                  // Initializes CAN-BUS Shield at specified baud rate.
      Serial.println("CAN-BUS Shield Initialized!");
      blink_led(2, 150, color[0]); //WHY is this blinking?
      is_CBS_init = true;
    }
    else
    {
      Serial.println("CAN-BUS Shield FAILED!");
      Serial.println("Retry initializing CAN-BUS Shield.");
      blink_led(3, 500, color[2]);
      delay(1000);
    }
  }


}



// Main Loop:
void loop() {
  unsigned char len = 0;
  unsigned char buf[8];

  if (CAN_MSGAVAIL == CAN.checkReceive()) {                // Checks for incoming data.
    CAN.readMsgBuf(&len, buf);                         // Reads incoming data. len: data length, buf[location]: actual data.
    unsigned long canId = CAN.getCanId();

    if (canId == 1536) {
      int rpmA = buf[0];
      int rpmB = buf[1];
      int ENGINE_RPM = ((rpmA * 256) + rpmB);
      String ALPHA_RPM = String(ENGINE_RPM / 10);
      ledStrip_update(ENGINE_RPM);/*
          alpha0.writeDigitAscii(0, ALPHA_RPM[0]);
          alpha0.writeDigitAscii(1, ALPHA_RPM[1]);
          alpha0.writeDigitAscii(2, ALPHA_RPM[2]);
          alpha0.writeDigitAscii(3, ALPHA_RPM[3]);
          alpha0.writeDisplay();*/
    }

    if (canId == 1550) {
      int gearA = buf[2];
      int gearB = buf[3];
      String ALPHA_GEAR = String((gearA * 256) + gearB - 2);
      if (ALPHA_GEAR.equals("0")) {
        alpha1.writeDigitAscii(0, 'N');
        alpha1.writeDigitAscii(1, ' ');
        alpha1.writeDigitAscii(2, ' ');
        alpha1.writeDigitAscii(3, ' ');
        alpha1.writeDisplay();
      } else if (ALPHA_GEAR.equals("1")) {
        alpha1.writeDigitAscii(0, ALPHA_GEAR[0]);
        alpha1.writeDigitAscii(1, ' ');
        alpha1.writeDigitAscii(2, ' ');
        alpha1.writeDigitAscii(3, ' ');
        alpha1.writeDisplay();
      } else if (ALPHA_GEAR.equals("2")) {
        alpha1.writeDigitAscii(0, ' ');
        alpha1.writeDigitAscii(1, ALPHA_GEAR[0]);
        alpha1.writeDigitAscii(2, ' ');
        alpha1.writeDigitAscii(3, ' ');
        alpha1.writeDisplay();
      } else if (ALPHA_GEAR.equals("3")) {
        alpha1.writeDigitAscii(0, ' ');
        alpha1.writeDigitAscii(1, ' ');
        alpha1.writeDigitAscii(2, ALPHA_GEAR[0]);
        alpha1.writeDigitAscii(3, ' ');
        alpha1.writeDisplay();
      } else if (ALPHA_GEAR.equals("4")) {
        alpha1.writeDigitAscii(0, ' ');
        alpha1.writeDigitAscii(1, ' ');
        alpha1.writeDigitAscii(2, ' ');
        alpha1.writeDigitAscii(3, ALPHA_GEAR[0]);
        alpha1.writeDisplay();
      }
    }

    if (canId == 1542) {
      int tempA = buf[2];
      int tempB = buf[3];
      String ALPHA_TEMP = String(((tempA * 256) + tempB) / 10);
      alpha2.writeDigitAscii(0, ALPHA_TEMP[0]);
      alpha2.writeDigitAscii(1, ALPHA_TEMP[1]);
      alpha2.writeDigitAscii(2, ALPHA_TEMP[2]);
      alpha2.writeDigitAscii(3, 'C');
      alpha2.writeDisplay();
    }

    if (canId == 1552) {
      int speedA = buf[4];
      int speedB = buf[5];
      String ALPHA_SPEED = String(((speedA * 256) + speedB) / 100);
      alpha0.writeDigitAscii(0, ALPHA_SPEED[0]);
      alpha0.writeDigitAscii(1, ALPHA_SPEED[1]);
      alpha0.writeDigitAscii(2, ALPHA_SPEED[2]);
      alpha0.writeDigitAscii(3, 'M');
      alpha0.writeDisplay();
    }

    if (canId == 1544) {
      int eopA = buf[0];
      int eopB = buf[1];
      int INT_EOP = ((((eopA * 256) + eopB) - 921) * 0.0145037738);
      String ALPHA_EOP = String((((eopA * 256) + eopB) - 921) * 0.0145037738);
      alpha2.writeDigitAscii(0, ALPHA_EOP[0]);
      alpha2.writeDigitAscii(1, ALPHA_EOP[1]);
      alpha2.writeDigitAscii(2, ALPHA_EOP[2]);
      alpha2.writeDigitAscii(3, 'P');
      alpha2.writeDisplay();
    }
  }
}



// LED Strip Update
void ledStrip_update(uint16_t LED_RPM) {
  unsigned long currentMillis = millis();
  if (LED_RPM >= shiftPT[0] && LED_RPM < shiftPT[1]) { //if the RPM is between the activation pt and the shift pt
    //map the RPM values to 9(really 8 since the shift point and beyond is handled below)and constrain the range
    int rpmMapped = map(LED_RPM, shiftPT[0], shiftPT[1], 0, 16);
    int rpmConstrained = constrain(rpmMapped, 0, 16);

    if (prev_range != rpmConstrained) { //This makes it so we only update the LED when the range changes so we don't readdress the strip every reading
      prev_range = rpmConstrained;
      clearStrip();
      for (int ledNum = 0; ledNum <= rpmConstrained; ledNum++) {
        if (ledNum <= ledStages[1]) {
          strip.setPixelColor(ledNum, color[0]);
        }
        else if (ledNum > ledStages[1] && ledNum <= ledStages[2]) {
          strip.setPixelColor(ledNum, color[1]);
        }
        else if (ledNum > ledStages[2] && ledNum < strip.numPixels()) {
          strip.setPixelColor(ledNum, color[2]);
        }
      }
      strip.show();
    }
  } else if (LED_RPM >= shiftPT[1]) { //SHIFT DAMNIT!! This blinks the LEDS back and forth with no delay to block button presses
    prev_range = 16;
    if (currentMillis - prevBlinkTime > blinkInterval) {
      prevBlinkTime = currentMillis;

      if (warningState == 0) {
        warningState = 1;
        for (int i = 0; i < strip.numPixels(); i = i + 2) {
          strip.setPixelColor(i, color[2]);
        }
        for (int i = 1; i < strip.numPixels(); i = i + 2) {
          strip.setPixelColor(i, 0);
        }
      } else {
        warningState = 0;
        for (int i = 1; i < strip.numPixels(); i = i + 2) {
          strip.setPixelColor(i, color[2]);
        }
        for (int i = 0; i < strip.numPixels(); i = i + 2) {
          strip.setPixelColor(i, 0);
        }
      }
      strip.show();
    }
  } else {
    if (prev_range != 10) {
      prev_range = 10;
      clearStrip();
    }
  }
}



// LED Strip Clear:
void clearStrip() {
  for (int ledNum = ledStages[0]; ledNum <= strip.numPixels(); ledNum++) {
    strip.setPixelColor(ledNum, 0);
  }
  strip.show();
}



// First LED Blink:
void blink_led(int count, int ms_delay, int colorInt) {
  clearStrip();
  strip.setBrightness(stripBrightness / 4);
  for (int i = 0; i < count; i++) {
    strip.setPixelColor(0, colorInt);
    strip.show();
    delay(ms_delay);
    clearStrip();
    delay(ms_delay / 2);
  }
  strip.setBrightness(stripBrightness);
  clearStrip();
}
