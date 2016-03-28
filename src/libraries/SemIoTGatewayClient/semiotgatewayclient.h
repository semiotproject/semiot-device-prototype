#ifndef SEMIOTGATEWAYCLIENT_H
#define SEMIOTGATEWAYCLIENT_H
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define UDP_GTW_OK_SIZE 5
#define UDP_GTW_OK_WORD "GTWOK"
#define UDP_GTW_PING_WORD "GTW"
#define DEBUG_LED_LIGHT LOW
#define DEBUG_LED_DARK HIGH

class SemIoTGatewayClient
{
public:
    SemIoTGatewayClient(WiFiUDP *udp, int debugLedPin = -1);
    ~SemIoTGatewayClient();
    void connectToSemIoTGateway();
    void connectToWps();
    void gtwSearch();
private:
    WiFiUDP *_udp;
    IPAddress _gatewayIp;
    int _debugLedPin;
    bool _debugLed;
    bool _debug;
    byte _mac[6];
    int _udpPort = 33333;
    char _gtwOkBuffer[UDP_GTW_OK_SIZE];
};

#endif // SEMIOTGATEWAYCLIENT_H
