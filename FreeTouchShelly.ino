#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <FT6X36.h>
#include <WiFi.h> // Wifi support
#include <HTTPClient.h>
#include <FS.h>       // Filesystem support header
#include <SPIFFS.h>   // Filesystem support header
#include <ArduinoJson.h> // Using ArduinoJson to read and write config files

// Uncomment this to use a piezo buzzer (you must specify the pin where the speaker is connected)
//#define speakerPin 26
// Define the storage to be used. For now just SPIFFS.
#define FILESYSTEM SPIFFS

TFT_eSPI tft = TFT_eSPI();

FT6X36 ts(&Wire, 27); // 27 is needed for the FreeTouchDeck

struct Wificonfig
{
  char ssid[64];
  char password[64];
  char hostname[64];
  uint8_t attempts;
  uint16_t attemptdelay;
};
Wificonfig wificonfig;

#include "TFT_eSPI_Button_Custom.h"

struct Coords {
  int16_t x;
  int16_t y;
};

struct BoundingBox {
  Coords c0;
  Coords c1;
};

struct ColourButton {
  TFT_eSPI_Button_Custom button;
  uint16_t colour;
  String text;
  bool pressed;
};

struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

#include "misc.h"
#include "Shelly.h"

std::vector<ColourButton> colourButtons = {
  {TFT_eSPI_Button_Custom(), TFT_RED,     "Red",     false},
  {TFT_eSPI_Button_Custom(), TFT_GREEN,   "Green",   false},
  {TFT_eSPI_Button_Custom(), TFT_NAVY,    "Navy",    false},
  {TFT_eSPI_Button_Custom(), TFT_BLUE,    "Blue",    false},
  {TFT_eSPI_Button_Custom(), TFT_PURPLE,  "Purple",  false},
  {TFT_eSPI_Button_Custom(), TFT_MAGENTA, "Magenta", false},
  {TFT_eSPI_Button_Custom(), TFT_ORANGE,  "Orange",  false},
  {TFT_eSPI_Button_Custom(), TFT_YELLOW,  "Yellow",  false},
  {TFT_eSPI_Button_Custom(), TFT_WHITE,   "White",   false},
};

TFT_eSPI_Button ledPowerButton;
TFT_eSPI_Button sleepButton;

BoundingBox colourGridBounds;
uint8_t gap = 4;

void setup() {
  Serial.begin(115200);

  Wire.begin();

  if (!ts.begin(40)) {
    Serial.println("[WARNING]: Unable to start the capacitive touchscreen");
  }
  else {
    Serial.println("[INFO]: Capacitive touch started");
  }

  tft.begin();
  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);

#ifdef speakerPin
  ledcSetup(2, 500, 8);
  Serial.println("[INFO]: Speaker enabled");
#endif

  int16_t max_x = tft.width() - 104;
  int16_t max_y = tft.height();
  colourGridBounds = {{0, 0}, {max_x, max_y}};

  //  drawRectThickness(bounds.c0.x, bounds.c0.y, bounds.c1.x, bounds.c1.y, BORDER_COLOUR, gap);
  drawColourGrid(colourGridBounds);

  ts.registerTouchHandler(touch);

  ledPowerButton.initButtonUL(&tft, 360, 20, 100, 100, TFT_WHITE, TFT_BLACK, TFT_WHITE, "Power", 4);
  sleepButton.initButtonUL(&tft, 360, 140, 100, 100, TFT_WHITE, TFT_BLACK, TFT_WHITE, "Sleep", 4);
  ledPowerButton.drawButton();
  sleepButton.drawButton();

  // -------------- Start filesystem ----------------------

  if (!FILESYSTEM.begin())
  {
    Serial.println("[ERROR]: SPIFFS initialisation failed!");
    drawErrorMessage("Failed to init SPIFFS! Did you upload the data folder?");
    while (1)
      yield(); // We stop here
  }
  Serial.println("[INFO]: SPIFFS initialised.");

  // Check for free space

  Serial.print("[INFO]: Free Space: ");
  Serial.println(SPIFFS.totalBytes() - SPIFFS.usedBytes());

  //------------------ Load Wifi Config ----------------------------------------------

  Serial.println("[INFO]: Loading Wifi Config");
  if (!loadWifiConfig()) {
    Serial.println("[WARNING]: Failed to load WiFi Credentials!");
  }
  else {
    Serial.println("[INFO]: WiFi Credentials Loaded");

    if (!startWifiOnly()) {
      Serial.println("[WARNING]: Could not connect to Wifi.");
    }
    else
    {
      Serial.print("[INFO]: Connected to Wifi (");
      Serial.print(WiFi.localIP());
      Serial.println(")");

      //      Serial.println("Updating shelly statuses"); // in the future this will auto-select the colour box on the grid
      //      shellyHandler.refreshState(rgbw2Shelly);
    }
  }
}

void beep() {
#ifdef speakerPin
  ledcAttachPin(speakerPin, 2);
  ledcWriteTone(2, 600);
  delay(50);
  ledcDetachPin(speakerPin);
  ledcWrite(2, 0);
#endif
}

void loop() {
  ts.loop();
}

void touch(TPoint p, TEvent e) {
  if (e != TEvent::TouchStart) {
    return;
  }

  //Flip things around so it matches our screen rotation
  //intentionally flipped for the FreeTouchDown display
  uint16_t t_x = p.y;
  uint16_t t_y = (uint16_t)map((long)p.x, (long)0, (long)tft.height(), (long)tft.height(), (long)0);

  Serial.printf("touch x:y - %d:%d (%d/%d)    -> %d:%d\n", p.x, p.y, tft.width(), tft.height(), t_x, t_y);

  boolean changed = false;
  boolean foundButton = false;

  for (auto & colourButton : colourButtons) {
    if (colourButton.button.contains(t_x, t_y)) {
      foundButton = true;

      if (!colourButton.pressed) {
        beep();
        Serial.printf("Button has been pressed - %d (%s)\n", colourButton.colour, colourButton.text);

        colourButton.button.press(true);
        colourButton.pressed = true;
        //          colourButton.button.setLabel("*");
        changed = true;
        RGB rgb = getRGB(colourButton.colour);
        setRgbShelly(rgbw2Shelly, rgb);
      }

      //        previousMillis = millis();
    } else {
      if (colourButton.pressed) {
        colourButton.button.press(false);
        colourButton.pressed = false;
        //          colourButton.button.setLabel("");
        changed = true;
      }
    }
  }

  if (!foundButton) {
    if (ledPowerButton.contains(t_x, t_y)) {
      toggleShelly(rgbw2Shelly);
    } else if (sleepButton.contains(t_x, t_y)) {
      Serial.println("I should be going to sleep now");
    }
  }

  if (changed) {
    Serial.println("Refreshing grid");
    drawColourGrid(colourGridBounds);
  }
}

void toggleShelly(Shelly shelly) {
  String url = shelly.getUrl();
  url += "?turn=toggle";

  const char* urlChars = url.c_str();
  String res = httpGETRequest(urlChars);
  Serial.println("Response:");
  Serial.println(res);
}

void setRgbShelly(Shelly shelly, RGB rgb) {
  String url = shelly.getUrl();
  url += "?turn=on&red=" + String(rgb.r) + "&green=" + String(rgb.g) + "&blue=" + String(rgb.b);

  const char* urlChars = url.c_str();
  String res = httpGETRequest(urlChars);
  Serial.println("Response:");
  Serial.println(res);
}

// draw a rectangular line of a given thickness/weight
void drawRectThickness(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colour, uint8_t thickness) {
  uint8_t count = 0;
  while (count < thickness) {
    tft.drawRect(x + count, y + count, w - (count * 2), h - (count * 2), colour);
    count++;
  }
}

RGB getRGB(uint16_t hexValue) {
  struct RGB rgbColor;
  rgbColor.r = ((((hexValue >> 11) & 0x1F) * 527) + 23) >> 6;
  rgbColor.g = ((((hexValue >> 5) & 0x3F) * 259) + 33) >> 6;
  rgbColor.b = (((hexValue & 0x1F) * 527) + 23) >> 6;
  return rgbColor;
}

void drawColourGrid(BoundingBox bounds) {
  int8_t w = 100;
  int8_t h = 100;

  int16_t min_x = bounds.c0.x + gap;
  int16_t min_y = bounds.c0.y + gap;
  int16_t max_x = bounds.c1.x - gap;
  int16_t max_y = bounds.c1.y - gap;

  int16_t x = min_x;
  int16_t y = min_y;

  int8_t col = 0;
  int8_t row = 0;

  for (auto & colourButton : colourButtons) {
    x = min_x + ((col * w) + (col * gap));
    if ((x + w) > max_x) {
      col = 0;
      x = min_x;
      row++;
    }
    y = min_y + ((row * h) + (row * gap));
    //    tft.fillRect(x, y, w, h, colourButton.colour);

    if (!colourButton.button.isInitialised()) {
      colourButton.button.initButtonUL(&tft, x, y, w, h, TFT_WHITE, colourButton.colour, TFT_WHITE, "*", 4);
    }
    if (colourButton.pressed) {
      Serial.printf("  %s is pressed!\n", colourButton.text);
    }
    colourButton.button.drawButton();

    col++;
  }
}
