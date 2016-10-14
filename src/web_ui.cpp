#include <Arduino.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>

#include "debug.h"
#include "web_ui.h"

WebUiTask::WebUiTask(SerialTask &serial) :
  server(80),
  ws("/ws"),
  serial(serial),
  MicroTasks::Task()
{
}

void WebUiTask::setup()
{
  MDNS.addService("http", "tcp", 80);

  // Setup the static files
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // Add the Web Socket server
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.onNotFound(onNotFound);

  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      DBUGF("BodyStart: %u", total);
    DBUGF("%s", (const char*)data);
    if(index + len == total)
      DBUGF("BodyEnd: %u", total);
  });

  server.begin();

  serial.onReadLine(onSerialReadLine, this);
}

unsigned long WebUiTask::loop(MicroTasks::WakeReason reason)
{
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

void WebUiTask::onSerialReadLine(uint8_t *sbuf, size_t len, bool binary, void *pvData)
{
  WebUiTask *self = (WebUiTask *)pvData;
  if(binary) {
    self->ws.binaryAll(sbuf, len);
  } else {
    self->ws.textAll(sbuf, len);
  }
}
