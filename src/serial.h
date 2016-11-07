#ifndef __SERIAL_H
#define __SERIAL_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <MicroTasks.h>
#include <MicroTasksTask.h>

// The amount of data to buffer before forcing a read event
#define SERIAL_BUFFER_SIZE 512

// Max time to wait before sending a read event
#define SERIAL_TIMEOUT 50

typedef void (* onReadLineCallback)(uint8_t *sbuf, size_t len, bool binary, void *clientData);

typedef enum {
  SerialParity_Odd,
  SerialParity_Even,
  SerialParity_None,
  SerialParity_MAX
} SerialParity;

class SerialClient;

class SerialTask : public MicroTasks::Task
{
private:
  SerialClient *clients;

  uint8_t lineBuffer[SERIAL_BUFFER_SIZE];
  size_t bufferPos = 0;
  long bufferReadTimeout;
  bool bufferIsBinary;

  unsigned long baud;
  SerialConfig config;
public:
  SerialTask();
  SerialTask(unsigned long baud);
  SerialTask(unsigned long baud, SerialConfig config);
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);

  void onReadLine(onReadLineCallback callback, void *clientData);

  unsigned long getBaud() {
    return baud;
  }

  int getDataBits();
  SerialParity getParity();
  int getStopBits();
};

class SerialClient
{
  friend SerialTask;
private:
  onReadLineCallback fnReadLineCallback;
  void *pvReadLineData;
public:
  SerialClient *next;

  SerialClient(onReadLineCallback callback, void *clientData);
};

#endif // __SERIAL_H
