#ifndef __SERIAL_H
#define __SERIAL_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <MicroTasks.h>
#include <MicroTasksTask.h>

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 5
#define SERIAL_BUFFER_SIZE 512
#define SERIAL_TIMEOUT 50

typedef void (* onReadLineCallback)(uint8_t *sbuf, size_t len, bool binary, void *clientData);

class SerialTask : public MicroTasks::Task
{
private:
  static WiFiServer server;
  static WiFiClient serverClients[];

  onReadLineCallback fnReadLineCallback;
  void *pvReadLineData;

  uint8_t lineBuffer[SERIAL_BUFFER_SIZE];
  size_t bufferPos = 0;
  long bufferReadTimeout;
  bool bufferIsBinary;
public:
  SerialTask();
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);

  void onReadLine(onReadLineCallback callback, void *clientData);
};

#endif // __SERIAL_H
