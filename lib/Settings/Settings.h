#ifndef SETTINGS_H
#define SETTINGS_H
#include <FileSystem.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WiFi.h>
class Settings{
public:
  bool init();
  bool storeSettings(String s);
  bool loadSettings();
  String returnSettings();
  bool factoryReset();

  String WiFiNetworks;

  char wlanSSID[50];
  char wlanPASS[50];
  //Soft AP settings
  char apSSID[50];
  char apPass[50];
  //Static IP Settings
  bool dhcpEnabled = true;
  IPAddress staticIP;
  IPAddress defaultGateway;
  IPAddress subnetMask;
  IPAddress primaryDNS;
  IPAddress secondaryDNS;


  char defaultHTML[50];

  bool buzzerEnabled;

  int interface = 0;
  //0 Ethernet
  //1 RS485
  //2 S3B

  int deviceID = 0;

private:
  FileSystem fileSystem;
  String discoveredNetworks;
  void setPublicVariables(JsonObject& settingsJSON);
  String loadedSettings;
};
#endif
