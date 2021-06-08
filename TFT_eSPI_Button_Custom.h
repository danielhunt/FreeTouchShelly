
#include "TFT_eSPI_Button_Override.h"

//cannot extend or 'friend' the base TFT_eSPI_Button class because all properties in that class are private. Need to work around that limitation here :(
class TFT_eSPI_Button_Custom: public TFT_eSPI_Button_Override {
 public:
  uint16_t getX();
  uint16_t getY();
  uint16_t getW();
  uint16_t getH();

 protected:
  uint8_t getCornerRadius();
};

uint8_t TFT_eSPI_Button_Custom::getCornerRadius() {
  return min(_w, _h) / 10; // Corner radius
}

uint16_t TFT_eSPI_Button_Custom::getX() { return _x1; }
uint16_t TFT_eSPI_Button_Custom::getY() { return _y1; }
uint16_t TFT_eSPI_Button_Custom::getW() { return _w; }
uint16_t TFT_eSPI_Button_Custom::getH() { return _h; }
