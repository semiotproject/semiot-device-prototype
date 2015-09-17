/******************************************************************************
SparkFunESP8266UDP.h
ESP8266 WiFi Shield Library Client Header File
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

#ifndef _SPARKFUNESP8266UDP_H_
#define _SPARKFUNESP8266UDP_H_

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <IPAddress.h>
#include <Udp.h>
#include "SparkFunESP8266WiFi.h"

#define UDP_TX_PACKET_MAX_SIZE 24

class ESP8266UDP : public UDP {
private:
    uint16_t _local_port; // local port to listen on
    uint16_t  _socket; // mux link id
    //bool ipMuxEn;
    uint8_t getFirstSocket();

public:
  ESP8266UDP();  // Constructor
  virtual uint8_t begin(uint16_t local_port);	// initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
  virtual void stop();  // Finish with the UDP socket

  // Sending UDP packets

  // Start building up a packet to send to the remote host specific in ip and port
  // Returns 1 if successful, 0 if there was a problem with the supplied IP address or port
  virtual int beginPacket(IPAddress ip, uint16_t port);
  // Start building up a packet to send to the remote host specific in host and port
  // Returns 1 if successful, 0 if there was a problem resolving the hostname or port
  virtual int beginPacket(const char *host, uint16_t port);
  // Finish off this packet and send it
  // Returns 1 if the packet was sent successfully, 0 if there was an error
  virtual int endPacket();
  // Write a single byte into the packet
  virtual size_t write(uint8_t c);
  // Write size bytes from buffer into the packet
  virtual size_t write(const uint8_t *buffer, size_t size);

  using Print::write;

  // Start processing the next available incoming packet
  // Returns the size of the packet in bytes, or 0 if no packets are available
  virtual int parsePacket();
  // Number of bytes remaining in the current packet
  virtual int available();
  // Read a single byte from the current packet
  virtual int read();
  // Read up to len bytes from the current packet and place them into buffer
  // Returns the number of bytes read, or 0 if none are available
  virtual int read(unsigned char* buffer, size_t len);
  // Read up to len characters from the current packet and place them into buffer
  // Returns the number of characters read, or 0 if none are available
  virtual int read(char* buffer, size_t len);
  // Return the next byte from the current packet without moving on to the next byte
  virtual int peek();
  virtual void flush();	// Finish reading the current packet

  // Return the IP address of the host who sent the current incoming packet
  virtual IPAddress remoteIP();
  // Return the port of the host who sent the current incoming packet
  virtual uint16_t remotePort();

  friend class WiFiDrv;
};

#endif
