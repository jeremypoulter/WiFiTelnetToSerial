#ifndef __WEB_UI_H
#define __WEB_UI_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <MicroTasks.h>
#include <MicroTasksTask.h>

#include "serial.h"

class WebUiTask : public MicroTasks::Task
{
private:
  AsyncWebServer server;
  AsyncWebSocket ws;
  SerialTask &serial;

  static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
  static void onNotFound(AsyncWebServerRequest *request);
  static void onSerialReadLine(uint8_t *sbuf, size_t len, bool binary, void *pvData);

public:
  WebUiTask(SerialTask &serial);
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);
};

#endif // __WEB_UI_H
