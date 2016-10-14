#include "telnet.h"
#include "debug.h"

TelnetClient::TelnetClient(TelnetTask* s, AsyncClient* c) :
  client(c),
  server(s),
  next(NULL),
  toSend(0)
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
  // DBUGF("TelnetClient::onPoll");
}

void TelnetClient::onAck(size_t len, uint32_t time)
{
  DBUGF("TelnetClient::onAck(%d, %d)", len, time);
  toSend -= len;
}

void TelnetClient::onError(int8_t error)
{
  DBUGF("TelnetClient::onError(%d)", error);
}

void TelnetClient::onTimeout(uint32_t time)
{
  DBUGF("TelnetClient::onTimeout(%d)", time);
  client->close();
}

void TelnetClient::onDisconnect()
{
  DBUGF("TelnetClient::onDisconnect");
  server->handleDisconnect(this);
}

void TelnetClient::onData(void *buf, size_t len)
{
  DBUGF("TelnetClient::onData(%p, %d)", buf, len);

  // TODO: Actually do telnet stuff, for now we will just write
  Serial.write((char *)buf, len);
}

size_t TelnetClient::write(const uint8_t *data, size_t len)
{
  while(!client->canSend()) {
    delay(10);
  }

  toSend += len;
  client->write((const char *)data, len);

  while(toSend > 0) {
    delay(10);
  }

  return len;
}
