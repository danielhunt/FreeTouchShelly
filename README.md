# FreeTouchShelly
A simple app for the [ESP32 TouchDown](https://github.com/DustinWatts/esp32-touchdown) to control a ShellyRGBW2 and a connected LED light strip.

Based loosely on Dustin Watt's [FreeTouchDeck](https://github.com/DustinWatts/FreeTouchDeck) for the ESP32 TouchDown - this is a complete change in direction for the UI for simplicity's sake.


# Hardware used

This project is built for the ESP32 TouchDown with capacitive touch:
The hardware I currently use is:

- ESP32 DEVKIT V1 (WROOM32) (**Partition scheme**: NO OTA with 2MB app and 2MB SPIFFS)
- 3.5" (480x320) TFT + Touchscreen with ILI9488 driver and FT6236 capacitive touch controller

# !- Library Dependencies -!
- Adafruit-GFX-Library (version 1.10.0 or higher), available through Library Manager
- TFT_eSPI (version 2.2.14 or higher), available through Library Manager
- FT6236 https://github.com/strange-v/FT6X36

# TFT_eSPI configuration

Before compiling and uploading the FreeTouchDeck.ino sketch, you will have to edit the **user_setup.h** file included with the TFT_eSPI library.

This can be found in your Arduino skechtbook folder under "libraries" - on Mac this is in `~/Documents/Arduino/libraries/TFT_eSPI`.

In `User_Setup.h` you need to uncomment the lines that apply to you hardware. For the ESP32 TouchDown I uncommented the `ILI9488_DRIVER` line.
