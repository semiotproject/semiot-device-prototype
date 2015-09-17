/******************************************************************************
SparkFunESP8266UDP.cpp
ESP8266 WiFi Shield Library Client Source File
Alexey Andreyev aa13q.ru
Original Creation Date: September 15, 2015
http://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library

!!! Description Here !!!

Development environment specifics:
	IDE: Arduino 1.6.5
	Hardware Platform: Arduino Uno
	ESP8266 WiFi Shield Version: 1.0

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include "SparkFunESP8266WiFi.h"
#include <Arduino.h>
#include "util/ESP8266_AT.h"
#include "SparkFunESP8266UDP.h"




uint8_t ESP8266UDP::getFirstSocket()
{
    esp8266.updateStatus();
    for (int i = 0; i < ESP8266_MAX_SOCK_NUM; i++)
    {
            if (esp8266._state[i] == AVAILABLE)
            {
                    return i;
            }
    }
    return ESP8266_SOCK_NOT_AVAIL;
    /*
    esp8266.updateStatus();
    for (int i = 0; i < ESP8266_MAX_SOCK_NUM; i++)
    {
            if (esp8266._status.ipstatus[i].linkID == 255)
            {
                    return i;
            }
    }
    return ESP8266_SOCK_NOT_AVAIL;
    */
}

ESP8266UDP::ESP8266UDP()
{
}

uint8_t ESP8266UDP::begin(uint16_t local_port)
{
    _socket = getFirstSocket();
    if (_socket != ESP8266_SOCK_NOT_AVAIL)
    {
        esp8266._state[_socket] = TAKEN;
        int16_t rsp = esp8266.udpConnect(_socket, "127.0.0.1", local_port, local_port, 2); // TODO: UDP mode
        return rsp;
    }
    _local_port = local_port;
}

void ESP8266UDP::stop()
{
    esp8266.close(_socket);
    esp8266._state[_socket] = AVAILABLE;
}

int ESP8266UDP::beginPacket(IPAddress ip, uint16_t port)
{
    char ipAddress[16];
    sprintf(ipAddress, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    //return esp8266.udpConnect(_socket,(const char *)ipAddress, port,_local_port,2);
    return beginPacket(ipAddress,port);
}

int ESP8266UDP::beginPacket(const char *host, uint16_t port)
{
    esp8266.udpConnect(_socket,host,port,_local_port,2); // TODO: UDP mode
}

int ESP8266UDP::endPacket()
{
}

size_t ESP8266UDP::write(uint8_t c)
{
    return write(&c, 1);
}

size_t ESP8266UDP::write(const uint8_t *buffer, size_t size)
{
    // TODO: separate to utils:
    IPAddress ip = remoteIP();
    char ipAddress[16];
    sprintf(ipAddress, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return esp8266.udpSend(_socket,buffer,size,ipAddress,remotePort());
}

int ESP8266UDP::parsePacket()
{
    return available();
}

int ESP8266UDP::available()
{
    if (_socket != ESP8266_SOCK_NOT_AVAIL)
    {
        int available = esp8266.available();
        if (available == 0)
        {
            // Delay for the amount of time it'd take to receive one character
            delayMicroseconds((1 / esp8266.getBaud()) * 10 * 1E6);
            // Check again just to be sure:
            available = esp8266.available();
        }
        return esp8266.available();
    }
    return 0;
}

int ESP8266UDP::read()
{
    return esp8266.read();
}

int ESP8266UDP::read(char *buffer, size_t len) {
    return read((unsigned char*)buffer, len);
}

int ESP8266UDP::read(unsigned char* buffer, size_t len) {
    int a = esp8266.available();
    if (a == 0)
    {
        return a;
    }
    else {
        if (len<a) {
            a=len;
        }
        for (int i=0; i<a; i++)
        {
                buffer[i] = esp8266.read();
        }
    }
    return a;
}

int ESP8266UDP::peek()
{
    return esp8266.peek();
}

void ESP8266UDP::flush()
{
    esp8266.flush();
}

IPAddress ESP8266UDP::remoteIP()
{
    return esp8266.remoteIP(_socket);
}

uint16_t ESP8266UDP::remotePort()
{
    return esp8266.remotePort(_socket);
}
