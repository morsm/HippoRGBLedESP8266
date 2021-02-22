#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

void sendUdpPacket(IPAddress addr, uint16_t port, char *payload)
{
  WiFiUDP Udp;
  
  Udp.beginPacket(addr, port);
  Udp.write(payload);
  Udp.endPacket();
}

void broadcastUdpPacket(uint16_t port, char *payload)
{
  IPAddress local = WiFi.localIP(), broadcast = local;
  broadcast[3] = 255;

  WiFiUDP Udp;
  
  Udp.beginPacketMulticast(broadcast, port, local);
  Udp.write(payload);
  Udp.endPacket();

}

IPAddress getBroadcastAddress()
{
  IPAddress local = WiFi.localIP();
  local[3] = 255;
  
  return local;
}
