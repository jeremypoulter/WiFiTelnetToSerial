#include <Arduino.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

#include "debug.h"
#include "web_ui.h"
#include "config.h"

WebUiTask *WebUiTask::self = NULL;

WebUiTask::WebUiTask(SerialTask &serial, WiFiManagerTask &wifi) :
  server(80),
  ws("/ws"),
  scanCompleteEvent(this),
  serial(serial),
  wifi(wifi),
  reboot(false),
  enableCors(true),
  MicroTasks::Task()
{
  wifi.onScanComplete(scanCompleteEvent);

  self = this;
}

void WebUiTask::setup()
{
  MDNS.addService("http", "tcp", 80);

  // Setup the static files
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // Add the Web Socket server
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/json");
    handleCors(response);

    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["id"] = ESP.getChipId();
    root["heap"] = ESP.getFreeHeap();
    root["version"] = ESCAPEQUOTE(VERSION);

    root.printTo(*response);
    request->send(response);
  });

  server.on("/serial", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/json");
    handleCors(response);

    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["baud"] = self->serial.getBaud();
    root["dataBits"] = self->serial.getDataBits();
    SerialParity parity = self->serial.getParity();
    root["parity"] = SerialParity_None == parity ? "N" :
                     SerialParity_Odd == parity ? "O" :
                     SerialParity_Even == parity ? "E" :
                     "UNKNOWN";
    root["stopBits"] = self->serial.getStopBits();

    root.printTo(*response);
    request->send(response);
  });

  server.on("/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if(NULL == self->scanRequest)
    {
      DBUGF("Start WiFi scan");
      self->scanRequest = request;
      self->wifi.StartScan();
    }
    else
    {
      request->send(400, "text/json", "{\"msg\":\"Busy\"}");
    }
  });

  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/json");
    handleCors(response);

    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["mac"] = WiFi.macAddress();
    root["localIP"] = WiFi.localIP().toString();
    root["subnetMask"] = WiFi.subnetMask().toString();
    root["gatewayIP"] = WiFi.gatewayIP().toString();
    root["dnsIP"] = WiFi.dnsIP().toString();
    wl_status_t status = WiFi.status();
    root["status"] = WL_NO_SHIELD == status ? "WL_NO_SHIELD" :
                     WL_IDLE_STATUS == status ? "WL_IDLE_STATUS" :
                     WL_NO_SSID_AVAIL == status ? "WL_NO_SSID_AVAIL" :
                     WL_SCAN_COMPLETED == status ? "WL_SCAN_COMPLETED" :
                     WL_CONNECTED == status ? "WL_CONNECTED" :
                     WL_CONNECT_FAILED == status ? "WL_CONNECT_FAILED" :
                     WL_CONNECTION_LOST == status ? "WL_CONNECTION_LOST" :
                     WL_DISCONNECTED == status ? "WL_DISCONNECTED" :
                     "UNKNOWN";
    root["hostname"] = WiFi.hostname();
    root["SSID"] = WiFi.SSID();
    root["BSSID"] = WiFi.BSSIDstr();
    root["RSSI"] = WiFi.RSSI();

    root.printTo(*response);
    request->send(response);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/json");
    handleCors(response);
    Config.serialize(*response);
    request->send(response);
  });

  server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/json");
    handleCors(response);
    response->print("{'msg':'done'}");
    request->send(response);
  });

  server.on("/settings", HTTP_DELETE, [](AsyncWebServerRequest *request) {
    Config.reset();
    self->reboot = true;
    MicroTask.wakeTask(self);

    AsyncResponseStream *response = request->beginResponseStream("text/json");
    handleCors(response);
    response->print("{'msg':'done'}");
    request->send(response);
  });

  server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/json");
    handleCors(response);
    response->print("{'msg':'done'}");
    request->send(response);

    self->reboot = true;
    MicroTask.wakeTask(self);
  });

  server.onNotFound(onNotFound);

#ifdef DEBUG
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index) {
      DBUGF("BodyStart: %u", total);
    }
    DBUGF("%s", (const char*)data);
    if(index + len == total) {
      DBUGF("BodyEnd: %u", total);
    }
  });
#endif

  server.begin();

  serial.onReadLine([](uint8_t *sbuf, size_t len, bool binary, void *data) { WebUiTask *self = (WebUiTask *)data; self->onSerialReadLine(sbuf, len, binary); }, this);
}

unsigned long WebUiTask::loop(MicroTasks::WakeReason reason)
{
  if(WakeReason_Event == reason)
  {
    int n = WiFi.scanComplete();
    if(NULL != WebUiTask::scanRequest && n != WIFI_SCAN_RUNNING)
    {
      DBUGF("WiFi scan complete");

      AsyncResponseStream *response = WebUiTask::scanRequest->beginResponseStream("text/json");
      handleCors(response);
      DynamicJsonBuffer jsonBuffer;

      JsonArray& root = jsonBuffer.createArray();
      DBUGF("%d networks found", n);
      for (int i = 0; i < n; ++i)
      {
        JsonObject& ssid = jsonBuffer.createObject();
        ssid["ssid"] = WiFi.SSID(i);
        int enc = WiFi.encryptionType(i);
        ssid["encryptionType"] = ENC_TYPE_WEP == enc ? "ENC_TYPE_WEP" :
                                 ENC_TYPE_TKIP == enc ? "ENC_TYPE_TKIP" :
                                 ENC_TYPE_CCMP == enc ? "ENC_TYPE_CCMP" :
                                 ENC_TYPE_NONE == enc ? "ENC_TYPE_NONE" :
                                 ENC_TYPE_AUTO == enc ? "ENC_TYPE_AUTO" :
                                 "UNKNOWN";
        ssid["rssi"] = WiFi.RSSI(i);
        root.add(ssid);
      }

      root.printTo(*response);
      WebUiTask::scanRequest->send(response);
      WebUiTask::scanRequest = NULL;
    }
  }

  if(reboot) {
    if(WakeReason_Manual == reason) {
      Serial.println("Rebooting...");
      return 100;
    }
    ESP.restart();
  }

  return MicroTask.Infinate;
}

void WebUiTask::onNotFound(AsyncWebServerRequest *request)
{
  DBUG("NOT_FOUND: ");
  if(request->method() == HTTP_GET)
    DBUGF("GET");
  else if(request->method() == HTTP_POST)
    DBUGF("POST");
  else if(request->method() == HTTP_DELETE)
    DBUGF("DELETE");
  else if(request->method() == HTTP_PUT)
    DBUGF("PUT");
  else if(request->method() == HTTP_PATCH)
    DBUGF("PATCH");
  else if(request->method() == HTTP_HEAD)
    DBUGF("HEAD");
  else if(request->method() == HTTP_OPTIONS)
    DBUGF("OPTIONS");
  else
    DBUGF("UNKNOWN");
  DBUGF(" http://%s%s", request->host().c_str(), request->url().c_str());

  if(request->contentLength()){
    DBUGF("_CONTENT_TYPE: %s", request->contentType().c_str());
    DBUGF("_CONTENT_LENGTH: %u", request->contentLength());
  }

  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    AsyncWebHeader* h = request->getHeader(i);
    DBUGF("_HEADER[%s]: %s", h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
  for(i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){
      DBUGF("_FILE[%s]: %s, size: %u", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
      DBUGF("_POST[%s]: %s", p->name().c_str(), p->value().c_str());
    } else {
      DBUGF("_GET[%s]: %s", p->name().c_str(), p->value().c_str());
    }
  }

  request->send(404);
}

void WebUiTask::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    DBUGF("ws[%s][%u] connect", server->url(), client->id());
    client->printf("Connected %u)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    DBUGF("ws[%s][%u] disconnect: %u", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    DBUGF("ws[%s][%u] error(%u): %s", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    DBUGF("ws[%s][%u] pong[%u]: %s", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      DBUGF("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      DBUGF("%s\n",msg.c_str());

      if(info->opcode == WS_TEXT) {
        Serial.printf("%s\n", msg.c_str());
      } else {
        // client->binary("I got your binary message");
      }
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          DBUGF("ws[%s][%u] %s-message start", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        DBUGF("ws[%s][%u] frame[%u] start[%llu]", server->url(), client->id(), info->num, info->len);
      }

      DBUGF("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }

      Serial.print(msg.c_str());

      if((info->index + len) == info->len) {
        DBUGF("ws[%s][%u] frame[%u] end[%llu]", server->url(), client->id(), info->num, info->len);
        if(info->final) {
          DBUGF("ws[%s][%u] %s-message end", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

void WebUiTask::onSerialReadLine(uint8_t *sbuf, size_t len, bool binary)
{
  if(binary) {
    ws.binaryAll(sbuf, len);
  } else {
    ws.textAll(sbuf, len);
  }
}

void WebUiTask::handleCors(AsyncResponseStream *response)
{
  if(self->enableCors) {
    response->addHeader("Access-Control-Allow-Origin", "*");
  }
}
