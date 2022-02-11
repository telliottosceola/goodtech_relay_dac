#include <Arduino.h>
#include <MCP23008.h>
#include <GPIOHandler.h>
#include <RGBLED.h>
#include <HTML_Handler.h>
#include <Settings.h>
#include <WiFiHandler.h>
#include <SPIFFS.h>
#include <MCP23008.h>
#include <mcp4725.h>
#include <ArduinoJson.h>
#include "HardwareSerial.h"
#include <S3B.h>

MCP23008 inputBoard;
GPIOHandler gpioHandler;
RGBLED rgbLED;
HTMLHandler httpHandler;
Settings settings;
WiFiHandler wifiHandler;

MCP23008 relay;
MCP4725 dac_one;
MCP4725 dac_two;

S3B module;

void inputChangeHandler(uint8_t input, uint8_t newState);
void backgroundTasks(void* pvParameters);
void buttonPressCallback(unsigned long duration);
void settingsUpdateCallback(bool success);

TaskHandle_t backgroundTask;
bool setupMode = false;
bool previousSetupMode = false;

String emailBuffer = "[]";
bool boot = true;
bool testMode = false;

void s3BReceiver(uint8_t* data, size_t len, uint8_t* transmitterAddress);
void s3BRSSI(int rssi, uint8_t* address);
void newSettings(char* newSettings, int len);
void s3BTransmitStatus(uint8_t status, uint8_t frameID);
void softwareS3BCommand();
void softwareATCommandResponse(uint8_t* data, size_t len);
void moduleSettingsLoaded();
void transmitStatus(uint8_t* transmitterAddress);

void tcpReceiver(uint8_t* buffer, size_t len);
void returnStatus();

void rs485Receiver(uint8_t* buffer, size_t len);

void inputChangeHandler(uint8_t input, uint8_t newState);
