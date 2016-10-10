#include <Arduino.h>
#include <ESP8266mDNS.h>

#include "debug.h"
#include "telnet.h"

using namespace MicroTasks;

TelnetTask::TelnetTask(SerialTask &serial) :
  server(TELNET_PORT),
  serial(serial),
  Task()
{
}

void TelnetTask::setup()
{
  server.onClient([](void *data, AsyncClient* c){ TelnetTask *telnet = (TelnetTask *)data; telnet->onClient(c); }, this);
  serial.onReadLine([](uint8_t *sbuf, size_t len, bool binary, void *data) { TelnetTask *telnet = (TelnetTask *)data; telnet->onSerialReadLine(sbuf, len, binary); }, this);

  server.begin();

  // Add service to MDNS-SD (mDNS started by ArduinoOTA)
  MDNS.addService("telnet", "tcp", 23);
}

unsigned long TelnetTask::loop(WakeReason reason)
{
  return MicroTask.Infinate;
}

void TelnetTask::onSerialReadLine(uint8_t *sbuf, size_t len, bool binary)
{
  for(TelnetClient *client = clients;
      client;
      client = client->next)
  {
    AsyncClient *socket = client->GetClient();
    socket->write((char *)sbuf, len);
  }
}

void TelnetTask::handleDisconnect(TelnetClient *client)
{
  // Remove from list;
  if(clients = client) {
    clients = client->next;
  }
  else
  {
    for(TelnetClient *c = clients; c; c = c->next)
    {
      if(c->next == client) {
        c->next = client->next;
        break;
      }
    }
  }

  // free memory
  delete client;
}

void TelnetTask::onClient(AsyncClient* c)
{
  if(c == NULL) {
    return;
  }
  TelnetClient *client = new TelnetClient(this, c);
  if(client)
  {
    c->setRxTimeout(3);
    client->next = clients;
    clients = client;
  }
  else
  {
    c->close(true);
    c->free();
    delete c;
  }
}
