#include <Arduino.h>
#include <ESP8266mDNS.h>

#include "debug.h"
#include "serial.h"

using namespace MicroTasks;

SerialTask::SerialTask() :
  bufferPos(0),
  bufferIsBinary(false),
  bufferReadTimeout(millis()),
  Task()
{
}

void SerialTask::setup()
{
  //start UART and the server
  Serial.begin(115200);
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

SerialClient::SerialClient(onReadLineCallback callback, void *clientData) :
  fnReadLineCallback(callback),
  pvReadLineData(clientData)
{
}
