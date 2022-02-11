#ifndef HTML_HANDLER_H
#define HTML_HANDLER_H
#include <Settings.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <SPIFFS.h>
class HTMLHandler{
public:
  void init(Settings &s);
  void loop();

  bool requestPending = false;

  void stop();

  void registerSettingsUpdateCallback(void(*settingsUpdateCallback)(bool success));

private:
  unsigned long commandReceiveTime = millis();
  const byte DNS_PORT = 53;
  DNSServer dnsServer;

  Settings *settings;
  void onRequest(AsyncWebServerRequest *request);
  void (*_settingsUpdateCallback)(bool success);
};
#endif
