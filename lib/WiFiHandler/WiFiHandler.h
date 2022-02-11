#ifndef WIFIHANDLER_H
#define WIFIHANDLER_H
#include <WiFi.h>
#include <Settings.h>
#include <ArduinoJson.h>
#include <RGBLED.h>
class WiFiHandler{
public:
  void init(Settings &s, RGBLED &r);
  bool checkWiFi(bool setupMode);
  void scanNetworks();

  String macAddress;
  char macBytes[18];
  char moduleIPString[17];
  IPAddress moduleIP;
private:
  Settings *settings;
  RGBLED *rgbLED;
  void(*_onWiFiConnectionProgressCallback)();
  uint8_t button = 32;
  unsigned long buttonPressTime;
  bool buttonPressed;
  bool setupButtonTimeout = 3000;
  unsigned long factoryResetTimeout = 5000;
  bool factoryResetEnable = false;
  bool previousButtonState;
};

#endif
