// ESP-8266 WiFi-module based
// Impulse counter
#include "SemIoTGatewayClient.h"

// Valtec VLF-R-I: "VLFR"
// Incotex Mercury M201: "M201"
#define MODEL_WORD "VLFR"
#define PULSE_PIN 2 // GPIO2

bool _debug = false;
#define SERIAL_BAUDRATE 115200 // debug port
// debug led to VCC:
#define DEBUG_LED_PIN 0 // GPIO0

#define MAX_COUNTER_LOW_NUMBER 10

WiFiUDP _udp;
int _udpPort = 33333;
SemIoTGatewayClient *semiotGtwClient;

uint16_t low_counter = 0; // 2 bytes
unsigned int high_counter = 0; // 4 bytes for esp8266
bool counter_changed = false;
bool need_to_reconnect = false;

void blink() {
    low_counter++;
    counter_changed = true;
}

void setup() {
    if (_debug) {
        Serial.begin(SERIAL_BAUDRATE);
        semiotGtwClient = new SemIoTGatewayClient(&_udp,_udpPort, &Serial,DEBUG_LED_PIN);
    }
    else {
        semiotGtwClient = new SemIoTGatewayClient(&_udp,_udpPort);
    }
    // TODO:
    //EEPROM.begin(16);
    //readCountersFromEeprom();
    pinMode(PULSE_PIN, INPUT);
    digitalWrite(PULSE_PIN,LOW);
    attachInterrupt(digitalPinToInterrupt(PULSE_PIN), blink, RISING);
    semiotGtwClient->connectToSemIoTGateway();
    if (_debug) {
        Serial.println("Setup completed, waiting for rising input");
    }
}

void loop() {
    if (need_to_reconnect==true) {
        need_to_reconnect = false;
        semiotGtwClient->connectToSemIoTGateway();
    }
    if (counter_changed==true) {
        counter_changed=false;
        if (low_counter==MAX_COUNTER_LOW_NUMBER) {
            low_counter=0;
            high_counter++;
        }
        // writeCountersToEeprom();
        // send some data
        if (WiFi.status() == WL_CONNECTED) {
            if (!_udp.beginPacket(semiotGtwClient->gatewayIp(), _udpPort)) {
                need_to_reconnect=true;
            }
            _udp.write(MODEL_WORD);
            _udp.write((low_counter >> 8) & 0xFF);
            _udp.write((low_counter >> 0) & 0xFF);

            _udp.write((high_counter >> 24) & 0xFF);
            _udp.write((high_counter >> 16) & 0xFF);
            _udp.write((high_counter >> 8) & 0xFF);
            _udp.write((high_counter >> 0) & 0xFF);
            byte *mac = semiotGtwClient->mac();
            // NOTE: danger access?
            _udp.write(mac[0]);
            _udp.write(mac[1]);
            _udp.write(mac[2]);
            _udp.write(mac[3]);
            _udp.write(mac[4]);
            _udp.write(mac[5]);
            if (_udp.endPacket()) {
                need_to_reconnect=true;
            }
            if (_debug) {
                Serial.print("counter = ");
                Serial.println(low_counter,DEC);
                Serial.print("high counter = ");
                Serial.println(high_counter,DEC);
            }
        }
        else {
            need_to_reconnect=true;
        }
    }
}
