// ESP-8266 WiFi-module based
// impulse counter
// which saves values value to EEPROM
// and sends current values via UDP
// to automatically recognized local server

#include "WiFiex.h"
#include "SemIoTGatewayClient.h" // GPIO 2

// Valtec VLF-R-I: "VLFR"
// Incotex Mercury M201: "M201"
char _modelWord[] = "VLFR";
char _idWord[] = "AAA1";

#define PULSE_PIN 2 // GPIO2

#define SERIAL_BAUDRATE 115200 // debug port
// debug led to VCC:
#define DEBUG_LED_LIGHT LOW
#define DEBUG_LED_DARK HIGH
#define DEBUG_LED_PIN -1 // GPIO0; -1 to disable
HardwareSerial *_debugSerial = NULL;// = &Serial;

WiFiUDP _udp;
int _udpPort = 33333;

unsigned int _counter = 0;// 4 bytes for esp8266
bool _counterChanged = false;
bool _needToReconnect = false;

SemIoTGatewayClient *semiotGtwClient;

void _iterate_counter() {
    _counter++;
    _counterChanged = true;
}

void setup() {
    if (_debugSerial) {
        _debugSerial->begin(SERIAL_BAUDRATE);
    }
    if (PULSE_PIN>-1) {
        pinMode(DEBUG_LED_PIN, OUTPUT);
    }
    semiotGtwClient = new SemIoTGatewayClient(&_udp,_udpPort,_debugSerial,DEBUG_LED_PIN);
    // TODO: EEPROM
    pinMode(PULSE_PIN, INPUT);
    digitalWrite(PULSE_PIN,LOW);
    attachInterrupt(digitalPinToInterrupt(PULSE_PIN), _iterate_counter, RISING);
    // TODO: remove it, repeating in loop():
    if (_debugSerial) {
        Serial.println("Trying to connect to WPS");
    }
    // _debugSerial,DEBUG_LED_PIN:
    connectToWPS(); // TODO: async via udp SoftAP
    if (_debugSerial) {
        Serial.println("Connected via WPS, searching for local server");
    }
    // semiotGtwClient->connectToSemIoTGateway();
    if (_debugSerial) {
        Serial.println("Setup completed");
    }
}

void loop() {
    /*
    if (_needToReconnect) {
        connectToWPS(NULL,DEBUG_LED_PIN); // TODO: async via udp SoftAP
    }
    */
    _counterChanged=true; // :trollface:
    semiotGtwClient->sendCounters(_modelWord,_idWord,&_counter,&_counterChanged,&_needToReconnect);
    delay(5000);
}
