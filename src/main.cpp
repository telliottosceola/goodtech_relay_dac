//#define S3B
#define TCP
// #define DEBUG
#include <Arduino.h>
#include <MCP23008.h>
#include <mcp4725.h>
#include <ArduinoJson.h>

#ifdef S3B
#include <S3B.h>
#endif

#ifdef TCP
#include "HardwareSerial.h"
#endif



MCP23008 relay;
MCP4725 dac_one;
MCP4725 dac_two;

#ifdef S3B
S3B module;
#endif
#ifdef TCP
HardwareSerial serial1(2);
#endif

#ifdef S3B
void s3BReceiver(uint8_t* data, size_t len, uint8_t* transmitterAddress);
void s3BRSSI(int rssi, uint8_t* address);
void newSettings(char* newSettings, int len);
void s3BTransmitStatus(uint8_t status, uint8_t frameID);
void softwareS3BCommand();
void softwareATCommandResponse(uint8_t* data, size_t len);
void moduleSettingsLoaded();
void transmitStatus(uint8_t* transmitterAddress);
#endif

#ifdef TCP
void tcpReceiver(uint8_t* buffer, size_t len);
void transmitStatus();
#endif

void inputChangeHandler(uint8_t input, uint8_t newState);

String devicesArray = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  relay.setRelays(4);
  relay.setInputs(255);
  relay.registerInputChangeCallback(inputChangeHandler);
  relay.init(10);

  dac_one.begin(false);
  dac_two.begin(true);

  #ifdef S3B
  module.init();
  module.receiveDataCallback(s3BReceiver);
  module.rssiUpdateCallback(s3BRSSI);
  module.transmissionStatusCallback(s3BTransmitStatus);
  module.softwareATCallback(softwareATCommandResponse);
  module.settingsLoadedCallback(moduleSettingsLoaded);
  #endif

  #ifdef TCP
  serial1.begin(115200);
  #endif
}

void loop() {
  #ifdef S3B
  module.loop();
  #endif
  #ifdef TCP
  if(serial1.available()){
    delay(20);
    uint8_t buffer[serial1.available()];
    serial1.read(buffer, sizeof(buffer));
    tcpReceiver(buffer, sizeof(buffer));
  }
  #endif
  relay.loop();
}

#ifdef S3B
void s3BReceiver(uint8_t* data, size_t len, uint8_t* transmitterAddress){
  bool transmit = false;
  Serial.println("Data Callback");
  delay(50);
  #ifdef DEBUG
  Serial.print("Received: ");
  for(int i = 0; i < len; i++){
    Serial.printf("%02X ", data[i]);
  }
  Serial.println();
  #endif
  DynamicJsonBuffer jBuffer;
  JsonObject& root = jBuffer.parseObject(data);
  if(root.success()){
    root.printTo(Serial);
    Serial.println();
    if(root.containsKey("relay_1")){
      transmit = true;
      Serial.println("Relay 1 command");
      if(root["relay_1"].as<bool>()){
        Serial.println("turn on relay 1");
        relay.turnOnRelay(1);
      }else{
        Serial.println("turn off relay 1");
        relay.turnOffRelay(1);
      }
    }
    if(root.containsKey("relay_2")){
      transmit = true;
      if(root["relay_2"].as<bool>()){
        relay.turnOnRelay(2);
      }else{
        relay.turnOffRelay(2);
      }
    }
    if(root.containsKey("relay_3")){
      transmit = true;
      if(root["relay_3"].as<bool>()){
        relay.turnOnRelay(3);
      }else{
        relay.turnOffRelay(3);
      }
    }
    if(root.containsKey("relay_4")){
      transmit = true;
      if(root["relay_4"].as<bool>()){
        relay.turnOnRelay(4);
      }else{
        relay.turnOffRelay(4);
      }
    }
    if(root.containsKey("dac_1")){
      transmit = true;
      dac_one.setOutputRaw(root["dac_1"].as<int>());
    }
    if(root.containsKey("dac_2")){
      transmit = true;
      dac_two.setOutputRaw(root["dac_2"].as<int>());
    }
    if(root.containsKey("operation")){
      char command[sizeof(root["operation"].as<char*>())];
      strcpy(command, root["operation"].as<char*>());
      if(strcmp(command,"get_status") == 0){
        transmit = true;
      }
    }
    if(transmitStatus){
      transmitStatus(transmitterAddress);
    }
  }
}

void transmitStatus(uint8_t* transmitterAddress){

  DynamicJsonBuffer jBuffer;

  Serial.println("get_status command");
  JsonObject& statusPacket = jBuffer.createObject();
  statusPacket["relay_1"] = relay.readRelayStatus(1);
  // Serial.println("2");
  statusPacket["relay_2"] = relay.readRelayStatus(2);
  statusPacket["relay_3"] = relay.readRelayStatus(3);
  statusPacket["relay_3"] = relay.readRelayStatus(4);
  // Serial.println("3");
  statusPacket["dac_1"] = dac_one.setPoint;
  // Serial.println("4");
  statusPacket["dac_2"] = dac_two.setPoint;
  size_t packetLen = statusPacket.measureLength()+1;
  char data[packetLen];
  statusPacket.printTo(data, packetLen);
  Serial.printf("%s\n", data);
  uint8_t destination[8];
  memcpy(destination,transmitterAddress,8);
  module.transmit(destination, (uint8_t*)data, packetLen);
}

void moduleSettingsLoaded(){

}

void s3BRSSI(int rssi, uint8_t* address){

}

void s3BTransmitStatus(uint8_t status, uint8_t frameID){

}

void softwareS3BCommand(){

}

void softwareATCommandResponse(uint8_t* data, size_t len){

}
#endif

#ifdef TCP
void tcpReceiver(uint8_t* buffer, size_t len){
  bool transmit = false;
  DynamicJsonBuffer jBuffer;
  JsonObject& root = jBuffer.parseObject(buffer);
  if(root.success()){
    root.printTo(Serial);
    Serial.println();
    if(root.containsKey("relay_1")){
      transmit = true;
      Serial.println("Relay 1 command");
      if(root["relay_1"].as<bool>()){
        Serial.println("turn on relay 1");
        relay.turnOnRelay(1);
      }else{
        Serial.println("turn off relay 1");
        relay.turnOffRelay(1);
      }
    }
    if(root.containsKey("relay_2")){
      transmit = true;
      if(root["relay_2"].as<bool>()){
        relay.turnOnRelay(2);
      }else{
        relay.turnOffRelay(2);
      }
    }
    if(root.containsKey("relay_3")){
      transmit = true;
      if(root["relay_3"].as<bool>()){
        relay.turnOnRelay(3);
      }else{
        relay.turnOffRelay(3);
      }
    }
    if(root.containsKey("relay_4")){
      transmit = true;
      if(root["relay_4"].as<bool>()){
        relay.turnOnRelay(4);
      }else{
        relay.turnOffRelay(4);
      }
    }
    if(root.containsKey("dac_1")){
      transmit = true;
      dac_one.setOutputRaw(root["dac_1"].as<int>());
    }
    if(root.containsKey("dac_2")){
      transmit = true;
      dac_two.setOutputRaw(root["dac_2"].as<int>());
    }
    if(root.containsKey("operation")){
      char command[sizeof(root["operation"].as<char*>())];
      strcpy(command, root["operation"].as<char*>());
      if(strcmp(command,"get_status") == 0){
        transmit = true;
      }
    }
    if(transmitStatus){
      transmitStatus();
    }
  }else{
    char string[len];
    memcpy(string, buffer, len);
    String badRequest = String(string);
    Serial.println(badRequest);
  }
}

void transmitStatus(){

  DynamicJsonBuffer jBuffer;

  Serial.println("get_status command");
  JsonObject& statusPacket = jBuffer.createObject();
  statusPacket["relay_1"] = relay.readRelayStatus(1);
  // Serial.println("2");
  statusPacket["relay_2"] = relay.readRelayStatus(2);
  statusPacket["relay_3"] = relay.readRelayStatus(3);
  statusPacket["relay_4"] = relay.readRelayStatus(4);
  // Serial.println("3");
  statusPacket["dac_1"] = dac_one.setPoint;
  // Serial.println("4");
  statusPacket["dac_2"] = dac_two.setPoint;
  size_t packetLen = statusPacket.measureLength()+1;
  char data[packetLen];
  statusPacket.printTo(data, packetLen);
  Serial.printf("%s\n", data);
  serial1.write(data, sizeof(data));
}
#endif

void inputChangeHandler(uint8_t input, uint8_t newState){

}
