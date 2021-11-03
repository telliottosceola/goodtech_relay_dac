#include <Arduino.h>
#include <MCP23008.h>
#include <S3B.h>
#include <mcp4725.h>

// #define DEBUG

MCP23008 relay;
S3B module;
MCP4725 dac_one;
MCP4725 dac_two;

void s3BReceiver(uint8_t* data, size_t len, uint8_t* transmitterAddress);
void s3BRSSI(int rssi, uint8_t* address);
void newSettings(char* newSettings, int len);
void s3BTransmitStatus(uint8_t status, uint8_t frameID);
void softwareS3BCommand();
void softwareATCommandResponse(uint8_t* data, size_t len);
void inputChangeHandler(uint8_t input, uint8_t newState);
void moduleSettingsLoaded();
void transmitStatus(uint8_t* transmitterAddress);

String devicesArray = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  relay.setRelays(2);
  relay.setInputs(255);
  relay.registerInputChangeCallback(inputChangeHandler);
  relay.init(10);

  module.init();
  module.receiveDataCallback(s3BReceiver);
  module.rssiUpdateCallback(s3BRSSI);
  module.transmissionStatusCallback(s3BTransmitStatus);
  module.softwareATCallback(softwareATCommandResponse);
  module.settingsLoadedCallback(moduleSettingsLoaded);

  dac_one.begin(false);
  dac_two.begin(true);
}

void loop() {
  // put your main code here, to run repeatedly:
  module.loop();
  relay.loop();
}

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

void inputChangeHandler(uint8_t input, uint8_t newState){

}
