#ifndef __WIFI_MANAGER_H
#define __WIFI_MANAGER_H

#include <MicroTasks.h>
#include <MicroTasksTask.h>
#include <DNSServer.h>                // Required for captive portal

class WiFiManagerTask : public MicroTasks::Task
{
private:
  DNSServer dnsServer;                  // Create class DNS server, captive portal re-direct

  // Access Point SSID, password & IP address. SSID will be softAP_ssid + chipID to make SSID unique
  String softAP_ssid;
  String softAP_password;
  IPAddress apIP;
  IPAddress netMask;

  // WiFi Client
  String client_ssid;
  String client_password;
  String hostname;

  // Are we in client mode or AP
  bool client;
  long timeout;

  // WiFi connection LED state
  int wifiLedState;

  void startAP();
  unsigned long loopAP();
  void stopAP();

  void startClient();
  unsigned long loopClient();
  void stopClient();
public:
  WiFiManagerTask(String hostname, String ssid, String password);
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);
};

#endif //  __WIFI_MANAGER_H
