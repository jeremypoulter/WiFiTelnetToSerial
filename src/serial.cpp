#include <Arduino.h>
#include <ESP8266mDNS.h>

#include "debug.h"
#include "serial.h"

using namespace MicroTasks;

WiFiServer SerialTask::server(23);
WiFiClient SerialTask::serverClients[MAX_SRV_CLIENTS];

SerialTask::SerialTask() :
  fnReadLineCallback(NULL),
  pvReadLineData(NULL),
  bufferPos(0),
  bufferIsBinary(false),
  bufferReadTimeout(millis())
{
}

void SerialTask::setup()
{
  //start UART and the server
  Serial.begin(115200);

  server.begin();
  server.setNoDelay(true);

  // Add service to MDNS-SD (mDNS started by ArduinoOTA)
  MDNS.addService("telnet", "tcp", 23);
}

unsigned long SerialTask::loop(WakeReason reason)
{
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient())
  {
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        serverClients[i].write("Connected!\r\n");
        DBUG("New client: "); DBUGLN(i);
        break;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }

  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (serverClients[i] && serverClients[i].connected())
    {
      if(serverClients[i].available())
      {
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) Serial.write(serverClients[i].read());
      }
    }
  }

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
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()) {
        const uint8_t *sbuf = lineBuffer;
        serverClients[i].write(sbuf, bufferPos);
        yield();
      }
    }
    if(NULL != fnReadLineCallback) {
      fnReadLineCallback(lineBuffer, bufferPos, bufferIsBinary, pvReadLineData);
    }

    bufferPos = 0;
    bufferIsBinary = false;
  }

  return 0;
}

void SerialTask::onReadLine(onReadLineCallback callback, void *clientData)
{
  fnReadLineCallback = callback;
  pvReadLineData = clientData;
}
