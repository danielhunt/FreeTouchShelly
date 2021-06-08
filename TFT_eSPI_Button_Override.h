
//cannot extend or 'friend' the base TFT_eSPI_Button class because all properties in that class are private. Need to fully re-implement it here :(
class TFT_eSPI_Button_Override {
 public:
  TFT_eSPI_Button_Override(void);
  // "Classic" initButton() uses centre & size
  void     initButton(TFT_eSPI *gfx, int16_t x, int16_t y,
  uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
  uint16_t textcolor, char *label, uint8_t textsize);

  // New/alt initButton() uses upper-left corner & size
  void     initButtonUL(TFT_eSPI *gfx, int16_t x1, int16_t y1,
  uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
  uint16_t textcolor, char *label, uint8_t textsize);
  
  // Adjust text datum and x, y deltas
  void     setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum = MC_DATUM);
  
  void     drawButton(bool inverted = false, String long_name = "");
  bool     contains(int16_t x, int16_t y);

  void     press(bool p);
  bool     isPressed();
  bool     justPressed();
  bool     justReleased();

  bool     isInitialised();
  void     setLabel(char *label);

 protected:
  uint8_t getCornerRadius();
  int16_t  _x1, _y1; // Coordinates of top-left corner of button
  uint16_t _w, _h;   // Width and height of button
  char     _label[10]; // Button text is 9 chars maximum unless long_name used

  bool    initialised;

 private:
  TFT_eSPI *_gfx;
  int16_t  _xd, _yd; // Button text datum offsets (wrt centre of button)
  uint8_t  _textsize, _textdatum; // Text size multiplier and text datum for button
  uint16_t _outlinecolor, _fillcolor, _textcolor;

  bool  currstate, laststate; // Button states
};

/***************************************************************************************
** Code for the GFX button UI element
** Grabbed from Adafruit_GFX library and enhanced to handle any label font
***************************************************************************************/
TFT_eSPI_Button_Override::TFT_eSPI_Button_Override(void) {
  _gfx       = nullptr;
  _xd        = 0;
  _yd        = 0;
  _textdatum = MC_DATUM;
  _label[9]  = '\0';
}

// Classic initButton() function: pass center & size
void TFT_eSPI_Button_Override::initButton(
 TFT_eSPI *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h,
 uint16_t outline, uint16_t fill, uint16_t textcolor,
 char *label, uint8_t textsize)
{
  // Tweak arguments and pass to the newer initButtonUL() function...
  initButtonUL(gfx, x - (w / 2), y - (h / 2), w, h, outline, fill,
    textcolor, label, textsize);
}

// Newer function instead accepts upper-left corner & size
void TFT_eSPI_Button_Override::initButtonUL(
 TFT_eSPI *gfx, int16_t x1, int16_t y1, uint16_t w, uint16_t h,
 uint16_t outline, uint16_t fill, uint16_t textcolor,
 char *label, uint8_t textsize)
{
  _x1           = x1;
  _y1           = y1;
  _w            = w;
  _h            = h;
  _outlinecolor = outline;
  _fillcolor    = fill;
  _textcolor    = textcolor;
  _textsize     = textsize;
  _gfx          = gfx;
  strncpy(_label, label, 9);

  initialised = true;
}

// Adjust text datum and x, y deltas
void TFT_eSPI_Button_Override::setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum)
{
  _xd        = x_delta;
  _yd        = y_delta;
  _textdatum = datum;
}

//re-implement this function so as to change corner radius on the button
void TFT_eSPI_Button_Override::drawButton(bool inverted, String long_name) {
  uint16_t fill, outline, text;

  if(!inverted) {
    fill    = _fillcolor;
    outline = _outlinecolor;
    text    = _textcolor;
  } else {
    fill    = _textcolor;
    outline = _outlinecolor;
    text    = _fillcolor;
  }

  // this line was modified to call a protected function instead of hardcoded private values
  uint8_t r = getCornerRadius(); // Corner radius
  _gfx->fillRoundRect(_x1, _y1, _w, _h, r, fill);
  //only draw the outline if the button is selected
  if (isPressed()) {
    _gfx->drawRoundRect(_x1, _y1, _w, _h, r, outline);
  }

  _gfx->setTextColor(text, fill);
  _gfx->setTextSize(_textsize);

  uint8_t tempdatum = _gfx->getTextDatum();
  _gfx->setTextDatum(_textdatum);
  uint16_t tempPadding = _gfx->getTextPadding();
  _gfx->setTextPadding(0);

  if (isPressed()) {
    if (long_name == "")
      _gfx->drawString(_label, _x1 + (_w/2) + _xd, _y1 + (_h/2) - 4 + _yd);
    else
      _gfx->drawString(long_name, _x1 + (_w/2) + _xd, _y1 + (_h/2) - 4 + _yd);
  }

  _gfx->setTextDatum(tempdatum);
  _gfx->setTextPadding(tempPadding);
}

bool TFT_eSPI_Button_Override::contains(int16_t x, int16_t y) {
  return ((x >= _x1) && (x < (_x1 + _w)) &&
          (y >= _y1) && (y < (_y1 + _h)));
}

void TFT_eSPI_Button_Override::press(bool p) {
  laststate = currstate;
  currstate = p;
}

bool TFT_eSPI_Button_Override::isPressed()    { return currstate; }
bool TFT_eSPI_Button_Override::justPressed()  { return (currstate && !laststate); }
bool TFT_eSPI_Button_Override::justReleased() { return (!currstate && laststate); }

bool TFT_eSPI_Button_Override::isInitialised() { return initialised; }

void TFT_eSPI_Button_Override::setLabel(char *label) {
  strncpy(_label, label, 9);
}

uint8_t TFT_eSPI_Button_Override::getCornerRadius() {
  return min(_w, _h) / 4; // Corner radius
}
