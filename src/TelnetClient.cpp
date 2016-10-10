#include "telnet.h"

TelnetClient::TelnetClient(TelnetTask* s, AsyncClient* c)
  : client(c)
  , server(s)
  , next(NULL)
{
  c->onError([](void *r, AsyncClient* c, int8_t error){ TelnetClient *req = (TelnetClient*)r; req->onError(error); }, this);
  c->onAck([](void *r, AsyncClient* c, size_t len, uint32_t time){ TelnetClient *req = (TelnetClient*)r; req->onAck(len, time); }, this);
  c->onDisconnect([](void *r, AsyncClient* c){ TelnetClient *req = (TelnetClient*)r; req->onDisconnect(); delete c; }, this);
  c->onTimeout([](void *r, AsyncClient* c, uint32_t time){ TelnetClient *req = (TelnetClient*)r; req->onTimeout(time); }, this);
  c->onData([](void *r, AsyncClient* c, void *buf, size_t len){ TelnetClient *req = (TelnetClient*)r; req->onData(buf, len); }, this);
  c->onPoll([](void *r, AsyncClient* c){ TelnetClient *req = (TelnetClient*)r; req->onPoll(); }, this);
}

TelnetClient::~TelnetClient()
{
}

void TelnetClient::onPoll()
{
}

void TelnetClient::onAck(size_t len, uint32_t time)
{
}

void TelnetClient::onError(int8_t error)
{
}

void TelnetClient::onTimeout(uint32_t time)
{
  client->close();
}

void TelnetClient::onDisconnect()
{
  server->handleDisconnect(this);
}

void TelnetClient::onData(void *buf, size_t len)
{
  // TODO: Actually do telnet stuff, for now we will just write
  Serial.write((char *)buf, len);
}
