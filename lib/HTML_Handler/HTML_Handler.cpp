#include <HTML_Handler.h>

#define WALLED_GARDEN
// #define DEBUG

AsyncWebServer controlServer(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");
IPAddress apIP(172, 217, 28, 1);

void HTMLHandler::init(Settings &s){
  // #ifdef DEBUG
  Serial.println("HTML Handler Init");

  // #endif
  settings = &s;

  Serial.print("Soft AP SSID:");
  Serial.print(settings->apSSID);

  WiFi.mode(WIFI_AP);
  delay(200);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(settings->apSSID, settings->apPass, 1, 0, 1);
  #ifdef WALLED_GARDEN
  dnsServer.setTTL(0);
  dnsServer.start(DNS_PORT, "*", apIP);
  #endif

  // controlServer.on("/update", std::bind(&HTMLHandler::configUpdate, this, std::placeholders::_1));
  controlServer.onNotFound(std::bind(&HTMLHandler::onRequest, this, std::placeholders::_1));
  controlServer.begin();
  #ifdef DEBUG
  Serial.println("HTMLHandler initialized");
  #endif
}

void HTMLHandler::loop(){
  dnsServer.processNextRequest();
}

void HTMLHandler::onRequest(AsyncWebServerRequest *request){
  #ifdef DEBUG
  Serial.println("New request to HTTPControl");
  Serial.printf("params: %i\n",request->params());
  Serial.printf("args: %i\n",request->args());
  Serial.println(request->url());
  #endif
  if(requestPending){
    return;
  }

  if(request->url() == "/loadSettings"){
    #ifdef DEBUG
    Serial.println("/loadSettings request");
    Serial.println("Settings: "+settings->returnSettings());
    #endif
    request->send(200, "text/plain", settings->returnSettings());
    return;
  }

  if(request->url() == "/status"){
    String allSettings = settings->returnSettings();
    DynamicJsonBuffer jBuffer;
    JsonObject& responseJSON = jBuffer.parseObject(allSettings);
    responseJSON.remove("destination_address");
    responseJSON.remove("transmit_interval");
    responseJSON.remove("difference_notify");
    String responseString;
    responseJSON.printTo(responseString);
    request->send(200, "text/plain", responseString);
    return;
  }

  if(request->url() == "/update"){
    if(request->args() > 0 && request->hasArg("body")){
      uint8_t zero = 0;
      String requestString = request->arg(zero);
      // #ifdef DEBUG
      Serial.println("New Settings: "+requestString);
      // #endif
      if(!settings->storeSettings(requestString)){
        #ifdef DEBUG
        Serial.println("Failed to store settings");
        #endif
        request->send(201, "text/plain","Settings update failed");
        _settingsUpdateCallback(false);
        return;
      }else{
        request->send(200, "text/plain","Settings updated");
        delay(500);
        stop();
        _settingsUpdateCallback(true);
        // ESP.restart();
      }
    }
    return;;
  }

  if(request->url() == "/Config"){
    request->send(SPIFFS, "/Config.html");
    return;
  }
  #ifdef DEBUG
  Serial.println("Sending Config");
  #endif
  request->send(SPIFFS, "/Config.html");
}

void HTMLHandler::stop(){
  #ifdef DEBUG
  Serial.println("Stop called");
  #endif
  controlServer.end();
  dnsServer.stop();
  WiFi.mode(WIFI_OFF);
}

void HTMLHandler::registerSettingsUpdateCallback(void(*settingsUpdateCallback)(bool success)){
  _settingsUpdateCallback = settingsUpdateCallback;
}
