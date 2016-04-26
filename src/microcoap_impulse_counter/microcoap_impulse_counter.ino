/*
* WARNING - UDP_TX_PACKET_MAX_SIZE is hardcoded by Arduino to 24 bytes
* This limits the size of possible outbound UDP packets
*/
#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include "endpoints.h"
#include "microcoap.h"

#define PORT 5683
#define PULSE_PIN 2 // GPIO2

String ap_ssid = "M201";
String ap_password = "Str0ngHack3rPa$$";

String sta_ssid = "ISST";
String sta_password = "Ikcx9Iaau4";

WiFiUDP udp;

uint8_t packetbuf[UDP_TX_PACKET_MAX_SIZE];
static uint8_t scratch_raw[32];
static coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};

int sz;
int rc;
coap_packet_t pkt;
int i;

struct savedData_t
{
    char light;
    uint16_t tick;
    String ap_ssid;
    String ap_password;
    String sta_ssid;
    String sta_password;
} device_data;

void iterateTick() {
    increment_tick();
}

void setup()
{
    ESP.eraseConfig();
    EEPROM_readAnything(0, device_data);
    device_data.ap_ssid = "M201";
    device_data.ap_password = "Str0ngHack3rPa$$";
    Serial.begin(115200);
    // attachInterrupt(digitalPinToInterrupt(PULSE_PIN), increment_tick, RISING);
    WiFi.mode(WIFI_AP_STA); // WIFI_AP, WIFI_STA, WIFI_AP_STA
    // TODO: while not connected
    WiFi.softAPConfig(device_data.ap_ssid, device_data.ap_password);
    WiFi.begin(sta_ssid.c_str(),sta_password.c_str());
    coap_setup();
    endpoint_setup();

    udp.begin(PORT);
    Serial.println("setup completed");
}

void udp_send(const uint8_t *buf, int buflen)
{
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    while(buflen--)
        udp.write(*buf++);
    udp.endPacket();
}

void loop()
{
    sz = 0;
    rc = 0;
    i = 0;
    if ((sz = udp.parsePacket()) > 0)
    {
        if (udp.available()) {
            udp.read(packetbuf, sz);
            for (i=0;i<sz;i++)
            {
                Serial.print(packetbuf[i], HEX);
                Serial.print(" ");
            }
            Serial.println("");
            if (0 != (rc = coap_parse(&pkt, packetbuf, sz)))
            {
                Serial.print("Bad packet rc=");
                Serial.println(rc, DEC);
            }
            else
            {
                size_t rsplen = sizeof(packetbuf);
                coap_packet_t rsppkt;
                coap_handle_req(&scratch_buf, &pkt, &rsppkt);
                memset(packetbuf, 0, rsplen);
                if (0 != (rc = coap_build(packetbuf, &rsplen, &rsppkt)))
                {
                    Serial.print("coap_build failed rc=");
                    Serial.println(rc, DEC);
                }
                else
                {
                    udp_send(packetbuf, rsplen);
                }
            }
        }
    }
}
