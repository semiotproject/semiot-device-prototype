#include "semiotgatewayclient.h"

SemIoTGatewayClient::SemIoTGatewayClient(WiFiUDP *udp, int debugLedPin):
    _udp(udp),
    _debugLedPin(debugLedPin)
{
    if (_bebugLedPing>=0) {
        _debugLed = true;
    }
    else {
        _debugLed = false;
    }
#if defined(DEBUG_ESP_PORT)
    _debug = true;
#else
    _debug = false;
#endif
}

SemIoTGatewayClient::~SemIoTGatewayClient()
{

}

void SemIoTGatewayClient::connectToSemIoTGateway()
{
    while (WiFi.status() != WL_CONNECTED) {
        connectToWps();
    }
    WiFi.macAddress(mac);
    gtwSearch();
}

void SemIoTGatewayClient::connectToWps()
{
    // WiFi.begin("",""); // decided to get rid of this
    // Long delay required especially soon after power on.
    delay(4000);
    // Check if WiFi is already connected and if not, begin the WPS process.
    if (WiFi.status() != WL_CONNECTED) {
        if (_debug) {
            DEBUG_ESP_PORT.println("\nAttempting connection ...");
        }
        if (_debugLed) {
            digitalWrite(DEBUG_LED_PIN, DEBUG_LED_LIGHT);
        }
        WiFi.beginWPSConfig();
        // Another long delay required.
        delay(3000);
        if (_debugLed) {
            digitalWrite(DEBUG_LED_PIN, DEBUG_LED_DARK);
        }
        if (WiFi.status() == WL_CONNECTED) {
            if (_debug) {
                DEBUG_ESP_PORT.println("Connected!");
                DEBUG_ESP_PORT.println(WiFi.localIP());
                DEBUG_ESP_PORT.println(WiFi.SSID());
                DEBUG_ESP_PORT.println(WiFi.macAddress());
            }
            if (_debugLed) {
                digitalWrite(DEBUG_LED_PIN, DEBUG_LED_LIGHT);
            }
        }
        else {
            if (_debug) {
                DEBUG_ESP_PORT.println("Connection failed!");
            }
            if (_debugLed) {
                digitalWrite(DEBUG_LED_PIN, DEBUG_LED_DARK);
            }
        }
    }
    else {
        if (_debug) {
            DEBUG_ESP_PORT.println("\nConnection already established.");
        }
        if (_debugLed) {
            digitalWrite(DEBUG_LED_PIN, DEBUG_LED_LIGHT);
        }
    }
}

void SemIoTGatewayClient::gtwSearch()
{
    _udp.begin(_udpPort);
    _gatewayIp = (~WiFi.subnetMask()) | WiFi.gatewayIP();
    // TODO: timeout
    for(;;) {
        if (_debug) {
            DEBUG_ESP_PORT.println("sending broadcast:");
            DEBUG_ESP_PORT.println(_gatewayIp[0],DEC);
            DEBUG_ESP_PORT.println(_gatewayIp[1],DEC);
            DEBUG_ESP_PORT.println(_gatewayIp[2],DEC);
            DEBUG_ESP_PORT.println(_gatewayIp[3],DEC);
        }
        if (_udp.beginPacket(_gatewayIp, udp_port)) {
            _udp.write(UDP_GTW_PING_WORD);
            _udp.endPacket();
            if (_debug) {
                DEBUG_ESP_PORT.println("...");
            }
            delay(1500);
            int sz = _udp.parsePacket();
            if (_debug) {
                DEBUG_ESP_PORT.print("sz=");
                DEBUG_ESP_PORT.println(sz,DEC);
            }
            if (sz==UDP_GTW_OK_SIZE) {
                _udp.read(_gtwOkBuffer,UDP_GTW_OK_SIZE);
                if (memcmp(_gtwOkBuffer, UDP_GTW_OK_WORD, UDP_GTW_OK_SIZE) == 0) {
                    if (_debug) {
                        DEBUG_ESP_PORT.println("found semiot-gateway!");
                    }
                    _gatewayIp=_udp.remoteIP();
                    break;
                }
            }
        }
    }
}
