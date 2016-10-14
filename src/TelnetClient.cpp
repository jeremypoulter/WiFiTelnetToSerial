#include "telnet.h"
#include "debug.h"

#ifdef DEBUG
# define TELCMDS
# define TELOPTS
#endif
#include "arpa_telnet.h"

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

  char *ptr = (char *)buf;
  while(len > 0 && IAC == ptr[0])
  {
    ptr++; len--;
    switch(ptr[0])
    {
      case DO:
      case DONT:
      case WILL:
      case WONT:
        DBUGF("Got IAC %s,%s", TELCMD(ptr[0]), TELOPT(ptr[1]));
        ptr += 2; len -= 2;
        break;
      case SB:
        DBUGF("Got IAC %s,%s", TELCMD(ptr[0]), TELOPT(ptr[1]));
        while (len > 2 && IAC != ptr[0] && SE != ptr[1]) {
          ptr++; len--;
        }
        break;
      case SE:
      case NOP:
      case BREAK:
      case AYT:
      case DM:
      case IP:
      case AO:
      case EC:
      case EL:
      case GA:
        DBUGF("Got IAC %s", TELCMD(ptr[0]));
        ptr++; len--;
        break;
      case IAC:
        DBUGLN("IAC");
        Serial.write(ptr, 1);
        ptr++; len--;
        break;
      default:
        DBUGF("Unknown code: %d", ptr[0]);
        break;
    }
  }

  if(len > 0) {
    Serial.write(ptr, len);
  }
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
