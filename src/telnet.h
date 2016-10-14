#ifndef __TELNET_H
#define __TELNET_H

#include <Arduino.h>
#include <ESPAsyncTCP.h>

#include <MicroTasks.h>
#include <MicroTasksTask.h>

#include "serial.h"

#define TELNET_PORT 23

class TelnetClient;

class TelnetTask : public MicroTasks::Task
{
  friend TelnetClient;
private:
  AsyncServer server;
  TelnetClient *clients;
  SerialTask &serial;

  void onSerialReadLine(uint8_t *sbuf, size_t len, bool binary);
  void onClient(AsyncClient* c);

  void handleDisconnect(TelnetClient *client);
public:
  TelnetTask(SerialTask &serial);
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);
};

class TelnetClient
{
  friend TelnetTask;
private:
  AsyncClient *client;
  TelnetTask *server;
  TelnetClient *next;
  volatile size_t toSend;

  void onPoll();
  void onAck(size_t len, uint32_t time);
  void onError(int8_t error);
  void onTimeout(uint32_t time);
  void onDisconnect();
  void onData(void *buf, size_t len);
public:
  TelnetClient(TelnetTask *s, AsyncClient *c);
  ~TelnetClient();

  AsyncClient *GetClient() {
    return client;
  }

  size_t write(const uint8_t *data, size_t len);
};

#endif // __TELNET_H
