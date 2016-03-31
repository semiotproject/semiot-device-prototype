// ESP-8266 WiFi-module based
// Impulse counter
// That save value to EEPROM
// And sends current values via UDP
// to automatically recognized local server

#include "WiFiex.h"
#include "SemIoTGatewayClient.h"

// Valtec VLF-R-I: "VLFR"
// Incotex Mercury M201: "M201"
char _modelWord[] = "VLFR";

#define PULSE_PIN 2 // GPIO2

bool _debug = true;
#define SERIAL_BAUDRATE 115200 // debug port
// debug led to VCC:
#define DEBUG_LED_LIGHT LOW
#define DEBUG_LED_DARK HIGH
#define DEBUG_LED_PIN 0 // GPIO0; -1 to disable
HardwareSerial *_debugSerial = &Serial;

WiFiUDP _udp;
int _udpPort = 33333;

unsigned int _counter = 0;// 4 bytes for esp8266
bool _counter_changed = false;
bool _need_to_reconnect = false;

SemIoTGatewayClient *semiotGtwClient;

void _iterate_counter() {
    _counter++;
    _counter_changed = true;
}

void setup() {

    if (_debugSerial) {
        _debugSerial->begin(SERIAL_BAUDRATE);
    }
    semiotGtwClient = new SemIoTGatewayClient(&_udp,_udpPort,_debugSerial,DEBUG_LED_PIN);
    // TODO: EEPROM
    pinMode(PULSE_PIN, INPUT);
    digitalWrite(PULSE_PIN,LOW);
    attachInterrupt(digitalPinToInterrupt(PULSE_PIN), _iterate_counter, RISING);
    if (_debugSerial) {
        Serial.println("Trying to connect to WPS");
    }
    while (WiFi.status() != WL_CONNECTED) {
        connectToWPS(_debugSerial,DEBUG_LED_PIN); // TODO: async via udp SoftAP
    }
    if (_debugSerial) {
        Serial.println("Connected via WPS, searching for local server");
    }
    semiotGtwClient->connectToSemIoTGateway();
    if (_debugSerial) {
        Serial.println("Setup completed");
    }
}

void loop() {
    if (_need_to_reconnect==true) {
        _need_to_reconnect = false;
        semiotGtwClient->connectToSemIoTGateway();
    }
    if (_counter_changed==true) {
        _counter_changed=false;
        // FIXME: check for overflow
        // TODO: writeCountersToEeprom();
        if (WiFi.status() == WL_CONNECTED) {
            if (!_udp.beginPacket(semiotGtwClient->gatewayIp(), _udpPort)) {
                _need_to_reconnect=true;
            }
            _udp.write(_modelWord);
            // TODO: separate to lib:
            _udp.write((_counter >> 24) & 0xFF);
            _udp.write((_counter >> 16) & 0xFF);
            _udp.write((_counter >> 8) & 0xFF);
            _udp.write((_counter >> 0) & 0xFF);
            byte *mac = semiotGtwClient->mac();
            // NOTE: danger access?
            _udp.write(mac[0]);
            _udp.write(mac[1]);
            _udp.write(mac[2]);
            _udp.write(mac[3]);
            _udp.write(mac[4]);
            _udp.write(mac[5]);
            if (!_udp.endPacket()) {
                _need_to_reconnect=true;
            }
            if (_debugSerial) {
                _debugSerial->print("counter = ");
                _debugSerial->println(_counter,DEC);
            }
        }
        else {
            _need_to_reconnect=true;
        }
    }
}
