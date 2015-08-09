#include <stdlib.h>
#include <stdio.h>

#include <Ticker.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <stdint.h>
#include <IPAddress.h> // FIXME: get rid of saving IP address into string

#include "microcoap.h"
#include "endpoints.h" // https://github.com/semiotproject/microcoap
#include "observers.h"
#include "wifisettings.h"

//#define UDP_TX_PACKET_MAX_SIZE 860 // FIXME: extern to 2048B or 8192?

// current client's host name and port
String HOST_NAME;
long unsigned int HOST_PORT = 5683;

uint8_t count=0;
bool count_changed;

Ticker ticker;
WiFiClient client;
WiFiUDP udp;

// FIXME: get rid of Strings:
String _cstrToString(char* buffer, unsigned int bufferPos)
{
    String item;
    unsigned int k;
    for (int k = 0; k < bufferPos; k++) {
        item += String(buffer[k]);
    }
    return item;
}

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
}

void _tick() {
    count++;
    count_changed=true;
}

void setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Serial.print("setup...\r\n");
    //setupESP8266();
    //ticker.attach(2, _tick); // 2 seconds
    ///coap_setup();
    //endpoint_setup();
    Serial.println("ready");
}

//FIXME: get rif of:
// TODO
IPAddress _ipAddressFromString(String hostname) {
    IPAddress _address; 
    os_sprintf((char*)hostname.c_str(), "%u.%u.%u.%u", _address[0], _address[1], &_address[2], _address[3]);
    return _address;
}

// FIXME: pointer to IPAddress maybe?
char* _ipAddresstoString(IPAddress _address)
{
    char szIPAddress[20];
    memset(szIPAddress, 0, sizeof(szIPAddress));
    sprintf(szIPAddress, "%u.%u.%u.%u", _address[0], _address[1], _address[2], _address[3]);
    return szIPAddress;
}

void _udp_send(const uint8_t *buf, int buflen, String host_name = HOST_NAME, long unsigned int host_port=HOST_PORT)
{
    udp.beginPacket(_ipAddressFromString(host_name), host_port);
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


void _sendCoAPpkt(coap_packet_t* pkt_p, String hostName = "", long unsigned int port = 0, bool addTick=false)
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
        _udp_send(buffer, rsplen);
        Serial.println("Answer sended");
    }
}

bool _coapUnsubscribe(coap_packet_t* pkt_p, String* hostName, long unsigned int* port) {
    coap_endpoint_path_t uri_path;
    _parse_uri_path_opt(pkt_p,&uri_path);
    return removeCoApObserver(hostName->c_str(), hostName->length(), port, &uri_path);
}

bool _coapSubscribe(coap_packet_t* pkt_p, String* hostName, long unsigned int* port)
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
        if (addCoAPObserver(hostName->c_str(), hostName->length(), port, rsppkt,&uri_path)) {
            Serial.println("Oberver added");
        }
        return true;
    }
    return false;
}

void _saveCurrentClientInfo() {
    HOST_NAME = _ipAddresstoString(udp.remoteIP());
    HOST_PORT = udp.remotePort();
}

// use this functions:

void listenCoAP()
{
    sz = udp.parsePacket();
    if (sz > 0) {
        // prepare coap answer:
	if (0 != (rc = coap_parse(&pkt, buffer, sz))) {
            Serial.print("Bad packet rc=");
            Serial.println(rc, DEC);
        }
        else {
            _saveCurrentClientInfo();
            // FIXME: BAD: UGLY: REWRITE!
            int o;
            
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
                    if (_coapSubscribe(&pkt, &HOST_NAME, &HOST_PORT)) {
                        Serial.println("subscribe ok");
                    }
                }
                else {
                    if ((val->len == 1) && (*val->p==1)) {
                        //TODO:
                        if (_coapUnsubscribe(&pkt,&HOST_NAME,&HOST_PORT)) {
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
            if ((count_changed) && is_coap_endpoint_path_t_eq(&observers[i].path,&path_dht)) {
                should_send=true;
		count_changed=false;
            }
            if (should_send) {
                _sendCoAPpkt(&observers[i].answer_draft_pkt, _cstrToString(observers[i].hostName, observers[i].hostNameLenght), observers[i].port,true);
            }
        }
    }
}

void loop()
{
    //TODO: add observers checker;
    //listenCoAP(); // listen always;
    //sendToObservers(); // sending only when apropriate data is changed
}
