#include <main.h>

HardwareSerial serial2(2);

void setup() {
  //Start USB serial connection
  Serial.begin(115200);

  //Start Spiffs
  if(!SPIFFS.begin(true)){
    #ifdef DEBUG
    Serial.println("SPIFFS Mount Failed");
    #endif
  }

  settings.loadSettings();

  //Initialize Relay controller
  relay.setRelays(4);
  relay.setInputs(255);
  relay.registerInputChangeCallback(inputChangeHandler);
  relay.init(10);

  //Initialize DAC
  dac_one.begin(false);
  dac_two.begin(true);

  //GPIO handler for config button
  gpioHandler.registerButtonPressCallback(buttonPressCallback);
  gpioHandler.init(settings, rgbLED);

  //Configuration web ui gpioHandler
  httpHandler.registerSettingsUpdateCallback(settingsUpdateCallback);

  //Background thread for CFG button and RGB LED.
  xTaskCreatePinnedToCore(backgroundTasks, "BackGround Tasks", 20000, NULL, 1, &backgroundTask, 1);

  //WiFi handler for AP Mode
  // wifiHandler.init(settings, rgbLED);
  // wifiHandler.scanNetworks();

  //RGB LED handler
  rgbLED.init(2,15,13,COMMON_ANODE, settings.buzzerEnabled);
  rgbLED.setMode(rgbLED.MODE_ALL_CLEAR);

  switch(settings.interface){
    case(0):{
      //Ethernet
      serial2.begin(115200);
      break;
    }
    case(1):{
      //RS485
      serial2.begin(57600);
      break;
    }
    case(2):{
      //S3B
      module.init();
      module.receiveDataCallback(s3BReceiver);
      module.rssiUpdateCallback(s3BRSSI);
      module.transmissionStatusCallback(s3BTransmitStatus);
      module.softwareATCallback(softwareATCommandResponse);
      module.settingsLoadedCallback(moduleSettingsLoaded);
      break;
    }
  }
}

void loop() {

  if(previousSetupMode != setupMode){
    previousSetupMode = setupMode;
    #ifdef DEBUG
    Serial.printf("SetupMode: %s\n",setupMode?"True":"False");
    #endif
    if(setupMode){
      if(WiFi.status() == WL_CONNECTED){
        #ifdef DEBUG
        Serial.println("Disconnecting WiFi");
        #endif
        WiFi.disconnect();
        while(WiFi.status() == WL_CONNECTED);
        #ifdef DEBUG
        Serial.println("Disconnected");
        #endif
      }
      rgbLED.setMode(rgbLED.MODE_SETUP);
      delay(1000);
      httpHandler.init(settings);
    }
  }
  if(setupMode){
    httpHandler.loop();
  }

  switch(settings.interface){
    case(0):{
      if(serial2.available()){
        delay(20);
        uint8_t buffer[serial2.available()];
        serial2.read(buffer, sizeof(buffer));
        tcpReceiver(buffer, sizeof(buffer));
      }
      break;
    }
    case(1):{
      if(serial2.available()){
        delay(20);
        uint8_t buffer[serial2.available()];
        serial2.read(buffer, sizeof(buffer));
        rs485Receiver(buffer, sizeof(buffer));
      }
      break;
    }
    case(2):{
      module.loop();
      break;
    }
  }
  relay.loop();
}

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
      returnStatus();
    }
  }else{
    char string[len];
    memcpy(string, buffer, len);
    String badRequest = String(string);
    Serial.println(badRequest);
  }
}

void rs485Receiver(uint8_t* buffer, size_t len){
  bool transmit = false;
  DynamicJsonBuffer jBuffer;
  JsonObject& root = jBuffer.parseObject(buffer);
  if(root.success()){
    root.printTo(Serial);
    Serial.println();
    if(!root.containsKey("address")){
      return;
    }
    if(root["address"].as<int>() != 0 && root["address"].as<int>() != settings.deviceID){
      return;
    }
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
      returnStatus();
    }
  }else{
    char string[len];
    memcpy(string, buffer, len);
    String badRequest = String(string);
    Serial.println(badRequest);
  }
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

void returnStatus(){

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
  serial2.write(data, sizeof(data));
}

void buttonPressCallback(unsigned long duration){
  if(duration > 50){
    #ifdef DEBUG
    Serial.printf("Button pressed for %ims\n",(int)duration);
    #endif
  }
  if(!setupMode){
    setupMode = true;
    return;
  }else{
    if(duration <= 4000){
      setupMode = false;
      httpHandler.stop();
      rgbLED.setMode(rgbLED.MODE_BOOT);
    }else{
      rgbLED.setMode(rgbLED.RANDOM);
      unsigned long start = millis();
      while(millis() < start+2000){
        rgbLED.loop();
      }
      settings.factoryReset();
      ESP.restart();
      httpHandler.stop();
      setupMode = false;
      rgbLED.setMode(rgbLED.MODE_BOOT);
      ESP.restart();
    }
  }

}

void settingsUpdateCallback(bool success){
  if(success){
    setupMode = false;
    ESP.restart();
  }
}

void backgroundTasks(void* pvParameters){
  for(;;){
    rgbLED.loop();
    gpioHandler.loop();
    vTaskDelay(10);
  }
  vTaskDelete( NULL );
}

void inputChangeHandler(uint8_t input, uint8_t newState){

}
