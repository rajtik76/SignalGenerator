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

#include <Adafruit_SSD1306.h>
#include <Debounce.h>

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

#define BUTTON_OK 2
#define BUTTON_RIGHT 3
Debounce okButton(BUTTON_OK);
Debounce rightButton(BUTTON_RIGHT);

char frequency[9] = "05000000"; // frequency

bool editMode = false;          // edit mode flag
uint8_t editPosition = 1;       // edit number position 1 - 8

// init display
void initDisplay(void) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
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

void displayMegaHertz(uint8_t markPosition) {
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

// display frequency
void displayFrequency(void) {
  long longFrequency = atol(frequency);
  if (longFrequency < 1000) {
    displayHertz();
  } else if (longFrequency < 1000000)
  {
    displayKiloHertz();
  } else
  {
    displayMegaHertz(0);
  }
}

// change edit position
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

// increase edit position and display
void increaseEditPositionAndDisplay(void) {
  changeEditPosition(false);
  displayMegaHertz(editPosition);
}

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Clear the buffer
  display.clearDisplay();

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();

  displayFrequency();
}

void loop() {
  // OK button pressed
  if (okButton.pressed()) {
    editPosition = 1;
    editMode = !editMode;

    if (editMode) {
      displayMegaHertz(editPosition);
    } else {
      displayFrequency();
    }
  }

  if (editMode && rightButton.pressed()) {
    increaseEditPositionAndDisplay();
  }
}