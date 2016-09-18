#ifndef __SERIAL_H
#define __SERIAL_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <MicroTasks.h>
#include <MicroTasksTask.h>

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 5

class SerialTask : public MicroTasks::Task
{
private:
  static WiFiServer server;
  static WiFiClient serverClients[];

public:
  SerialTask();
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);
};

#endif // __SERIAL_H
