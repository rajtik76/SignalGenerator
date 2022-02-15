/**************************************************************************
  This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

  This example is for a 128x64 pixel display using I2C to communicate
  3 pins are required to interface (two I2C and one reset).

  Adafruit invests time and resources providing this open
  source code, please support Adafruit and open-source
  hardware by purchasing products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries,
  with contributions from the open source community.
  BSD license, check license.txt for more information
  All text above, and the splash screen below must be
  included in any redistribution.
 **************************************************************************/

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <AD9850SPI.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Encoder.h>
#include "Debounce.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins
#define BUTTON_SET 2
#define ROTARY_CLK 3
#define ROTARY_DT 4
#define ROTARY_BUTTON 5

// AD9850
#define W_CLK_PIN 13
#define FQ_UD_PIN 12
#define RESET_PIN 10

// Rotary encoder
Encoder myEnc(ROTARY_CLK, ROTARY_DT);
long rotaryPrevPosition = 0;
int8_t rotaryDirection = 0;   // 0 = no change, 1 = rotate CC, -1 = rotate CCW

Debounce rotaryButtonDebounce(ROTARY_BUTTON, HIGH);

// AD9850 trim frequency
double trimFreq = 124999000;

char frequency[9] = "00001000"; // frequency
char editFrequency[9];          // frequency in edit mode
uint8_t editPosition = 1;       // edit number position 1 - 8

// timers
unsigned long buttonSetTimer = millis();
unsigned long rotaryButtonTimer = millis();

// button flags
bool editMode = false;          // edit mode flag
bool buttonSetPressed = false;  // set button pressed flag
bool rotaryButtonPressed = false;

// rotary
int previousRotaryClkState;

// init display
void initDisplay(void) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
}

void displayHertz(void) {
  initDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.print("    ");
  display.print(frequency[5]);
  display.print(frequency[6]);
  display.print(frequency[7]);
  display.println();
  display.print("     Hz");

  display.display();
}

void displayKiloHertz(void) {
  initDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.print("  ");
  display.print(frequency[2]);
  display.print(frequency[3]);
  display.print(frequency[4]);
  display.print(".");
  display.print(frequency[5]);
  display.print(frequency[6]);
  display.print(frequency[7]);
  display.println();
  display.print("    kHz");

  display.display();
}

void displayMegaHertz(uint8_t markPosition, char *frequency) {
  initDisplay();

  for (uint8_t i = 0; i < 8; i++)
  {
    if (i == 2) { // add dot between MHz and Khz
      display.setTextColor(SSD1306_WHITE);
      display.print(".");
    }

    if (markPosition > 0 && (markPosition - 1) == i) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.print(frequency[i]);
  }

  display.setTextColor(SSD1306_WHITE);
  display.println();
  display.print("    MHz");

  display.display();
}

void displayFrequency(void) {
  long longFrequency = atol(frequency);

  if (longFrequency < 1000) {
    displayHertz();
  } else if (longFrequency < 1000000)
  {
    displayKiloHertz();
  } else
  {
    displayMegaHertz(0, frequency);
  }
}

void changeEditPosition(bool inverse) {
  if (inverse == false) {
    if (editPosition == 8) {
      editPosition = 1;
    } else {
      editPosition++;
    }
  } else {
    if (editPosition == 1) {
      editPosition = 8;
    } else {
      editPosition--;
    }
  }
}

void increaseEditPositionAndDisplay(void) {
  changeEditPosition(false);
  displayMegaHertz(editPosition, editFrequency);
}

void positionChangeValue(bool substraction) {
  char value = editFrequency[(editPosition - 1)];

  if (substraction) {
    if (value == '9') {
      value = '0';
    } else {
      value++;
    }
  } else {
    if (value == '0') {
      value = '9';
    } else {
      value--;
    }
  }

  editFrequency[editPosition - 1] = value;
  displayMegaHertz(editPosition, editFrequency);
}

void onButtonSetPress(void) {
  if (millis() - buttonSetTimer > 250) {
    buttonSetTimer = millis();
    editMode = !editMode;
    buttonSetPressed = true;    
  }
}

void onRotaryButtonPress(void) {
  if (millis() - rotaryButtonTimer > 250) {
    rotaryButtonTimer = millis();
    rotaryButtonPressed = true;
  }
}

// reset Arduino
void(* resetFunc) (void) = 0;

void setup() {
  Serial.begin(9600);

  // pins mode
  pinMode(BUTTON_SET, INPUT);
  pinMode(ROTARY_BUTTON, INPUT_PULLUP);

  // interrupts
  attachInterrupt(digitalPinToInterrupt(BUTTON_SET), onButtonSetPress, RISING);

  // AD9850 setup
  DDS.begin(W_CLK_PIN, FQ_UD_PIN, RESET_PIN);
  DDS.calibrate(trimFreq);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();

  displayFrequency();

  delay(500);
  DDS.up();
  DDS.setfreq(atol(frequency), 0);
}

void loop() {
  if (editMode && buttonSetPressed) { // enter to edit mode
    editPosition = 1;
    strcpy(editFrequency, frequency);
    displayMegaHertz(editPosition, editFrequency);

    buttonSetPressed = false;
  }

  if (!editMode && buttonSetPressed) { // exit edit mode
    if (strcmp(frequency, editFrequency) != 0) { // set frequency in AD9850

      Serial.print("Frequency was changed from:");
      Serial.print(atol(frequency), DEC);
      Serial.print("Hz to:");
      Serial.print(atol(editFrequency), DEC);
      Serial.println("Hz");

      strcpy(frequency, editFrequency); // copy edit frequency to frequency variable

      DDS.up();
      DDS.setfreq(atol(frequency), 0);
    }
    
    displayFrequency();
    
    buttonSetPressed = false;
  }

  if (editMode) { // edit mode

    long rotaryNewPosition = round(myEnc.read() / 4);
    if (rotaryNewPosition != rotaryPrevPosition) { // rotary moved
      if (rotaryNewPosition > rotaryPrevPosition) {
        rotaryDirection = 1; // CC = increase
      } else {
        rotaryDirection = -1; // CCW = decrease
      }
      rotaryPrevPosition = rotaryNewPosition;
    } else {
      rotaryDirection = 0;
    }

    if (rotaryButtonDebounce.pressed()) { // rotary button pressed (next digit)
      increaseEditPositionAndDisplay();
    }

    if (rotaryDirection == 1) { // UP button pressed
      rotaryDirection = 0;
      positionChangeValue(true);
      if (false) {
        Serial.println("Frequency + EEPROM RESET");

        initDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.print("  RESETED");
        display.display();

        strcpy(editFrequency, {"00000000"});
        strcpy(frequency, {"00000000"});
        for (uint16_t i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.write(i, 0);
        }
        resetFunc(); // reset Arduino
      }
    }

    if (rotaryDirection == -1) { // DOWN button pressed
      rotaryDirection = 0;
      positionChangeValue(false);
    }
  }
}
