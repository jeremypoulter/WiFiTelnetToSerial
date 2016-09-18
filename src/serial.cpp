#include <Arduino.h>

#include "debug.h"
#include "serial.h"

using namespace MicroTasks;

WiFiServer SerialTask::server(23);
WiFiClient SerialTask::serverClients[MAX_SRV_CLIENTS];

SerialTask::SerialTask()
{
}

void SerialTask::setup()
{
  //start UART and the server
  //Serial.begin(115200);

  server.begin();
  server.setNoDelay(true);
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
        DEBUG.print("New client: "); DEBUG.println(i);
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

  //check UART for data
  if(Serial.available())
  {
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].write(sbuf, len);
        delay(1);
      }
    }
  }

  return 0;
}
