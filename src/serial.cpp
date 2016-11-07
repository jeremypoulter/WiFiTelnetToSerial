#include <Arduino.h>
#include <ESP8266mDNS.h>

#include "debug.h"
#include "serial.h"

using namespace MicroTasks;

SerialTask::SerialTask() :
  bufferPos(0),
  bufferIsBinary(false),
  bufferReadTimeout(millis()),
  baud(115200),
  config(SERIAL_8N1),
  Task()
{
}

SerialTask::SerialTask(unsigned long baud) :
  bufferPos(0),
  bufferIsBinary(false),
  bufferReadTimeout(millis()),
  baud(baud),
  config(SERIAL_8N1),
  Task()
{
}

SerialTask::SerialTask(unsigned long baud, SerialConfig config) :
  bufferPos(0),
  bufferIsBinary(false),
  bufferReadTimeout(millis()),
  baud(baud),
  config(config),
  Task()
{
}

void SerialTask::setup()
{
  //start UART and the server
  Serial.begin(baud, config);
}

unsigned long SerialTask::loop(WakeReason reason)
{
  uint8_t i;

  // check UART for data, and read in to our buffer
  bool sendData = false;
  while(!sendData && bufferPos < SERIAL_BUFFER_SIZE && Serial.available())
  {
    uint8_t byte = Serial.read();
    lineBuffer[bufferPos++] = byte;

    // Detect end of line, need to handle single \r, \n, \r\n, \n\r but not \n\n
    // or \r\r
    if('\n' == byte || '\r' == byte)
    {
      uint8_t nextByte = Serial.peek();
      if(('\n' == nextByte || '\r' == nextByte) && byte != nextByte) {
        uint8_t byte = Serial.read();
        lineBuffer[bufferPos++] = byte;
      }

      sendData = true;
    }

    // Detect binary data
    if(byte >= 128) {
      bufferIsBinary = true;
    }

    bufferReadTimeout = millis() + SERIAL_TIMEOUT;
  }

  // Also send the data if we gone x ms without reading anything or is our buffer full
  if(bufferPos > 0 && (millis() > bufferReadTimeout || SERIAL_BUFFER_SIZE == bufferPos))
  {
    sendData = true;
  }

  if(sendData)
  {
    //push UART data to all connected telnet clients
    for(SerialClient *client = clients;
        client;
        client = client->next)
    {
      client->fnReadLineCallback(lineBuffer, bufferPos, bufferIsBinary, client->pvReadLineData);
    }

    bufferPos = 0;
    bufferIsBinary = false;
  }

  return 0;
}

void SerialTask::onReadLine(onReadLineCallback callback, void *clientData)
{
  SerialClient *client = new SerialClient(callback, clientData);
  if(client) {
    client->next = clients;
    clients = client;
  }
}

int SerialTask::getDataBits()
{
  switch(config)
  {
    case SERIAL_5N1:
    case SERIAL_5N2:
    case SERIAL_5E1:
    case SERIAL_5E2:
    case SERIAL_5O1:
    case SERIAL_5O2:
      return 5;

    case SERIAL_6N1:
    case SERIAL_6N2:
    case SERIAL_6E1:
    case SERIAL_6E2:
    case SERIAL_6O1:
    case SERIAL_6O2:
      return 6;

    case SERIAL_7N1:
    case SERIAL_7N2:
    case SERIAL_7E1:
    case SERIAL_7E2:
    case SERIAL_7O1:
    case SERIAL_7O2:
      return 7;

    case SERIAL_8N1:
    case SERIAL_8N2:
    case SERIAL_8E1:
    case SERIAL_8E2:
    case SERIAL_8O1:
    case SERIAL_8O2:
      return 8;
  }

  return -1;
}

SerialParity SerialTask::getParity()
{
  switch(config)
  {
    case SERIAL_5N1:
    case SERIAL_5N2:
    case SERIAL_6N1:
    case SERIAL_6N2:
    case SERIAL_7N1:
    case SERIAL_7N2:
    case SERIAL_8N1:
    case SERIAL_8N2:
      return SerialParity_None;

    case SERIAL_5E1:
    case SERIAL_5E2:
    case SERIAL_6E1:
    case SERIAL_6E2:
    case SERIAL_7E1:
    case SERIAL_7E2:
    case SERIAL_8E1:
    case SERIAL_8E2:
      return SerialParity_Even;

    case SERIAL_5O1:
    case SERIAL_5O2:
    case SERIAL_6O1:
    case SERIAL_6O2:
    case SERIAL_7O1:
    case SERIAL_7O2:
    case SERIAL_8O1:
    case SERIAL_8O2:
      return SerialParity_Odd;
  }

  return SerialParity_MAX;
}

int SerialTask::getStopBits()
{
  switch(config)
  {
    case SERIAL_5N1:
    case SERIAL_6N1:
    case SERIAL_7N1:
    case SERIAL_8N1:
    case SERIAL_5E1:
    case SERIAL_6E1:
    case SERIAL_7E1:
    case SERIAL_8E1:
    case SERIAL_5O1:
    case SERIAL_6O1:
    case SERIAL_7O1:
    case SERIAL_8O1:
      return 1;

    case SERIAL_5N2:
    case SERIAL_6N2:
    case SERIAL_7N2:
    case SERIAL_8N2:
    case SERIAL_5E2:
    case SERIAL_6E2:
    case SERIAL_7E2:
    case SERIAL_8E2:
    case SERIAL_5O2:
    case SERIAL_6O2:
    case SERIAL_7O2:
    case SERIAL_8O2:
      return 2;
  }

  return -1;
}

SerialClient::SerialClient(onReadLineCallback callback, void *clientData) :
  fnReadLineCallback(callback),
  pvReadLineData(clientData)
{
}
