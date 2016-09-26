#ifndef __WEB_UI_H
#define __WEB_UI_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <MicroTasks.h>
#include <MicroTasksTask.h>

class WebUiTask : public MicroTasks::Task
{
private:
  AsyncWebServer server;
  AsyncWebSocket ws;

  static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
  static void onNotFound(AsyncWebServerRequest *request);

public:
  WebUiTask();
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);
};

#endif // __WEB_UI_H
