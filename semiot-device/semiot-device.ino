// https://github.com/semiotproject/microcoap
#include "microcoap.h"
#include "endpoints.h"
#include "observers.h"
// ESP8266 Arduino firmware
#ifdef ESP8266_ARDUINO_FIRMWARE

  #include <stdlib.h>
  #include <stdio.h>

  #include <Ticker.h>
  #include <SPI.h>
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
  #include <stdint.h>
  #include <IPAddress.h>

#endif // ESP8266_ARDUINO_FIRMWARE

// ESP8266_VIA_ARDUINO_SERIAL
#ifdef ESP8266_VIA_ARDUINO_SERIAL

  #include <SoftwareSerial.h>
  #include "SparkFunESP8266WiFi.h"
  #include "SparkFunESP8266UDP.h"
  #include <IPAddress.h>

#endif // ESP8266_VIA_ARDUINO_SERIAL

#include "wifisettings.h"

//#define UDP_TX_PACKET_MAX_SIZE 860 // FIXME: extern to 2048B or 8192?

// current client's host name and port
#define ip_str_max_lenght 16
char _host_buf[ip_str_max_lenght];
char* HOST_NAME=(char*)&_host_buf;
long unsigned int HOST_PORT = 5683;

uint8_t count=0;
//TODO: implement without ticker
/*
//bool count_changed;
Ticker ticker;
*/

#ifdef ESP8266_VIA_ARDUINO_SERIAL

//WiFiClient client;
ESP8266UDP udp;

#endif

#ifdef ESP8266_ARDUINO_FIRMWARE

//WiFiClient client;
WiFiUDP udp;

#endif

//TODO: move to microcoap.h:
// saving the uri path from pkt_p:
void _parse_uri_path_opt(coap_packet_t* pkt_p, coap_endpoint_path_t* dest) {
    unsigned int opt_count = pkt_p->numopts;
    unsigned int segm_count = 0;
    unsigned int opt_i;
    int segment_lengh=0;
    for (opt_i=0;opt_i<opt_count;opt_i++) {
        if (pkt_p->opts[opt_i].num==COAP_OPTION_URI_PATH) {
            segment_lengh = pkt_p->opts[opt_i].buf.len;
            int buflen = pkt_p->opts[opt_i].buf.len;
            const uint8_t* buf = pkt_p->opts[opt_i].buf.p;
            char segment[buflen];
            int segm_i = 0;
            while(buflen--) {
                uint8_t x = *buf++;
                char c = char(x);
                segment[segm_i]=c;
                segm_i++;
            }
            memcpy((char *)dest->elems[segm_count], (const char*)&segment, segment_lengh);
            segm_count+=1;
        }
    }
    dest->count=segm_count;
}

void setupESP8266()
{
    #ifdef ESP8266_ARDUINO_FIRMWARE
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // register udp
    udp.begin(COAP_PORT);
    #endif

    #ifdef ESP8266_VIA_ARDUINO_SERIAL
      // The ESP8266 can be set to one of three modes:
      //  1 - ESP8266_MODE_STA - Station only
      //  2 - ESP8266_MODE_AP - Access point only
      //  3 - ESP8266_MODE_STAAP - Station/AP combo
      // Use esp8266.getMode() to check which mode it's in:
      int retVal = esp8266.getMode();
      if (retVal != ESP8266_MODE_STA)
      { // If it's not in station mode.
        // Use esp8266.setMode([mode]) to set it to a specified
        // mode.
        retVal = esp8266.setMode(ESP8266_MODE_STA);
        if (retVal < 0)
        {
          Serial.println(F("Error setting mode."));
          // errorLoop(retVal); //TODO:
        }
      }
      Serial.println(F("Mode set to station"));

      // esp8266.status() indicates the ESP8266's WiFi connect
      // status.
      // A return value of 1 indicates the device is already
      // connected. 0 indicates disconnected. (Negative values
      // equate to communication errors.)
      retVal = esp8266.status();
      if (retVal <= 0)
      {
        Serial.print(F("Connecting to "));
        Serial.println(SSID);
        // esp8266.connect([ssid], [psk]) connects the ESP8266
        // to a network.
        // On success the connect function returns a value >0
        // On fail, the function will either return:
        //  -1: TIMEOUT - The library has a set 30s timeout
        //  -3: FAIL - Couldn't connect to network.
        retVal = esp8266.connect(SSID, PASSWORD);
        if (retVal < 0)
        {
          Serial.println(F("Error connecting"));
          // errorLoop(retVal); //TODO
        }
      }

    #endif
}

void _tick() {
    count++;
    //count_changed=true;
}

void setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Serial.print("setup...\r\n");
    setupESP8266();
    //TODO: implement without ticker
    //ticker.attach(2, _tick); // 2 seconds
    coap_setup();
    endpoint_setup();
    Serial.println("ready");
}

//FIXME: https://github.com/esp8266/Arduino/issues/488
//siscanf(hostname, "%d.%d.%d.%d\r\n", _address[0], _address[1], &_address[2], _address[3]);
IPAddress _ipAddressFromString(char* hostname) {
    IPAddress _address;
    String address = String(hostname);
    String octade;

    octade = address.substring(0,address.indexOf("."));
    _address[0] = octade.toInt();
    address = address.substring(address.indexOf(".")+1,address.length());

    octade = address.substring(0,address.indexOf("."));
    _address[1] = octade.toInt();
    address = address.substring(address.indexOf(".")+1,address.length());

    octade = address.substring(0,address.indexOf("."));
    _address[2] = octade.toInt();
    address = address.substring(address.indexOf(".")+1,address.length());

    octade = address.substring(0,address.indexOf("."));
    _address[3] = octade.toInt();
    address = address.substring(address.indexOf(".")+1,address.length());

    /*
    Serial.print(_address[0],DEC);
    Serial.print('.');
    Serial.print(_address[1],DEC);
    Serial.print('.');
    Serial.print(_address[2],DEC);
    Serial.print('.');
    Serial.println(_address[3],DEC);
    */

    return _address;
}

// FIXME: pointer to IPAddress maybe?
char* _ipAddresstoString(IPAddress _address)
{
    char szIPAddress[ip_str_max_lenght];
    memset(szIPAddress, 0, sizeof(szIPAddress));
    sprintf(szIPAddress, "%u.%u.%u.%u", _address[0], _address[1], _address[2], _address[3]);
    return szIPAddress;
}

void _udp_send(const uint8_t *buf, int buflen, char* host_name = HOST_NAME, long unsigned int host_port=HOST_PORT)
{
    udp.beginPacket(_ipAddressFromString((char*)host_name), host_port);
    while(buflen--)
	udp.write(*buf++);
    udp.endPacket();
}

// COAP with observe:

coap_packet_t pkt; // parse recieved coap packet
uint8_t buffer[UDP_TX_PACKET_MAX_SIZE] = {0}; // recieve and send buffer
uint32_t sz; // udp.parsePacket() size
int rc; // coap error codes


coap_packet_t rsppkt;
static uint8_t scratch_raw[UDP_TX_PACKET_MAX_SIZE];
static coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};
//size_t rsplen; //TODO:

void _addTickCountToOpt(coap_buffer_t* buf) {
    buf->p = &count;
    buf->len=sizeof(count);
}


void _sendCoAPpkt(coap_packet_t* pkt_p, char* hostName, long unsigned int port, bool addTick=false)
{
    size_t rsplen = sizeof(buffer);
    if (!addTick) {
        coap_handle_req(&scratch_buf, pkt_p, &rsppkt);
    }
    else {
	// FIXED:
        //rsppkt.opts[0].buf.p = &count;
        //rsppkt.opts[0].buf.len = sizeof(count);
	_addTickCountToOpt(&rsppkt.opts[0].buf);
        rsppkt.hdr.id[0]++;
        rsppkt.hdr.id[1]++;
        memcpy(&rsppkt, pkt_p, sizeof(pkt_p));
    }
    if (0 != (rc = coap_build(buffer, &rsplen, &rsppkt))) {
        Serial.print("coap_build failed rc=");
        Serial.println(rc, DEC);
    }
    else {
        _udp_send(buffer, rsplen, hostName, port);
        Serial.println("Answer sended");
    }
}

bool _coapUnsubscribe(coap_packet_t* pkt_p, char* hostName, long unsigned int* port) {
    coap_endpoint_path_t uri_path;
    _parse_uri_path_opt(pkt_p,&uri_path);
    return removeCoApObserver(hostName, port, &uri_path);
}

bool _coapSubscribe(coap_packet_t* pkt_p, char* hostName, long unsigned int* port)
{
    /*
    Serial.println("dump:");
                int buflen = pkt.opts[1].buf.len;
                const uint8_t* buf = pkt.opts[1].buf.p;
                char segment[buflen];
                int segm_i = 0;
                while(buflen--) {
                    uint8_t x = *buf++;

                    char c = char(x);
                    Serial.print(char(c));
                    Serial.print(" ");
                    segment[segm_i]=c;
                    segm_i++;
                }
                Serial.print("\n");
    */
    // saving uri_path before corrupting the pkt:
    coap_endpoint_path_t uri_path;
    _parse_uri_path_opt(pkt_p,&uri_path);

    //handling request:
    size_t rsplen = sizeof(buffer);
    coap_handle_req(&scratch_buf, pkt_p, &rsppkt);
    rsppkt.numopts=2;
    rsppkt.opts[1].num=rsppkt.opts[0].num;
    rsppkt.opts[1].buf.p = rsppkt.opts[0].buf.p;
    rsppkt.opts[1].buf.len = rsppkt.opts[0].buf.len;
    rsppkt.opts[0].num = COAP_OPTION_OBSERVE;
    // FIXED:
    //rsppkt.opts[0].buf.p = &count;
    //rsppkt.opts[0].buf.len = sizeof(count);
    _addTickCountToOpt(&rsppkt.opts[0].buf);
    if (0 != (rc = coap_build(buffer, &rsplen, &rsppkt))) {
        Serial.print("coap_build failed rc=");
        Serial.println(rc, DEC);
    }
    else {
        _udp_send(buffer, rsplen);
        Serial.println("Answer sended");
        //_tick(); // FIXME: should I really do it?
        rsppkt.hdr.t = COAP_TYPE_NONCON;
        /*
        Serial.println("uri path: ");
        Serial.println(uri_path.elems[0]);
        Serial.println(uri_path.elems[1]);
        */
        if (addCoAPObserver(hostName, port, rsppkt,&uri_path)) {
            Serial.println("Oberver added");
        }
        return true;
    }
    return false;
}

void _saveCurrentClientInfo() {
    memcpy(HOST_NAME, _ipAddresstoString(udp.remoteIP()), ip_str_max_lenght);
    HOST_PORT = udp.remotePort();
}

// use this functions:

void listenCoAP()
{
    sz = udp.parsePacket();
    if (sz > 0) {
	udp.read(buffer, sizeof(buffer));
	int i;

	for (i=0;i<sz;i++)
	{
	    Serial.print(buffer[i], HEX);
	    Serial.print(" ");
	}
	Serial.println("");

        // prepare coap answer:
	if (0 != (rc = coap_parse(&pkt, buffer, sz))) {
            Serial.print("Bad packet rc=");
            Serial.println(rc, DEC);
        }
        else {
            _saveCurrentClientInfo();
            // cheking for observe option:
            uint8_t optionNumber;
            coap_buffer_t* val = NULL;
            for (optionNumber = 0; optionNumber < pkt.numopts; optionNumber++) {
                coap_option_t* currentOption = &pkt.opts[optionNumber];
                if (currentOption->num == COAP_OPTION_OBSERVE) {
                    val = &currentOption->buf;
                    break;
                }
            }
            if (val != NULL) {
            // http://tools.ietf.org/html/draft-ietf-core-observe-16#section-2
                if (val->len == 0) { // register
		    if (_coapSubscribe(&pkt, (char*)&HOST_NAME, &HOST_PORT)) {
                        Serial.println("subscribe ok");
                    }
                }
                else {
                    if ((val->len == 1) && (*val->p==1)) {
                        //TODO:
                        if (_coapUnsubscribe(&pkt,(char*)&HOST_NAME,&HOST_PORT)) {
                            Serial.println("unsubscribe ok");
                        }
                    }
                }
            }
            else {
                _sendCoAPpkt(&pkt, HOST_NAME, HOST_PORT);
            }
        }
    }
}

void sendToObservers()
{
    int i;
    if (getObserversCount() > 0)
    {
        //Serial.print("Observers count: ");
        //Serial.println(getObserversCount(),DEC);
        for (i = 0; i < getObserversCount(); i++) {
            //Serial.print("sending to observer: ");
            //Serial.print(cstrToString(observers[i].hostName, observers[i].hostNameLenght));
            //Serial.print(":");
            //Serial.println(observers[i].port, DEC);
            bool should_send=false;
            unsigned int n = observers[i].path.count;
            unsigned n_i;
	    /*
            if ((count_changed) && is_coap_endpoint_path_t_eq(&observers[i].path,&path_dht)) {
                should_send=true;
		count_changed=false;
            }
            if (should_send) {
                _sendCoAPpkt(&observers[i].answer_draft_pkt, observers[i].hostName,observers[i].port,true);
            }
            */
	    //TODO:
        }
    }
}

void loop()
{
    //TODO: add observers checker;
    listenCoAP(); // listen always;
    sendToObservers(); // sending only when apropriate data is changed
}
