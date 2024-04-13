#include <Wire.h> //Libraries for OLED Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


// Using fancy symbols
// https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
#define INFO 0x2A //Star
#define SEND 0x1B //Arrow left
#define RECEIVE 0x1A //Arrow right
#define ECHO 0x12 //Arrow up and down

#define OLED_RST -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

int scrollTextStartY = 12;
String scrollText = "";

String title = "";

void showLog() {
  // Scroll text
  int lineCount = 1;
  for (char c : scrollText) {if (c == '\n') lineCount ++;}
  while (lineCount > 7) { //remove first lines until only 8 lines appear on screen
    //remove first lines
    int firstLineBreak = 0;
    while (scrollText[firstLineBreak] != '\n') {firstLineBreak ++;}
    scrollText = scrollText.substring(firstLineBreak + 1);
    lineCount--;
  }

  // Show UI
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(title);
  display.drawFastHLine(0, 10, 127,WHITE);

  // Show scrolled text
  display.setCursor(0,scrollTextStartY);
  display.print(scrollText);

  //update display
  display.display();
}

void logln(char mode, String newText) {
  time_t tnow = time(nullptr);
  scrollText += ctime(&tnow);
  scrollText += (String) mode + newText + "\n";
}

void screenPrintln(char mode, String newText) {
  logln(mode, newText);
  showLog();
}

void oledSetup(String setTitle) {
  title = setTitle;
  //setup OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.display();
  delay(200);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setTextWrap(false);
  display.cp437(true);
  display.clearDisplay();
}