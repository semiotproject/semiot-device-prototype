// ESP-8266 WiFi-module based
// RS-485 data listener
// which saves values to EEPROM
// and sends current values via UDP
// to automatically recognized local server

#include "WiFiex.h"
#include "SemIoTGatewayClient.h"
#include "EnergomeraCE102.h"

// EnergoMera-CE102R5-AK: "NRGM"
// Teplovodokhran Pulsar water consumption meter: "PLSR"
char _modelWord[] = "NRGM";

uint16_t deviceAddress = 1363;
HardwareSerial *_dataSerial = &Serial;
#define SERIAL_BAUDRATE 9600 // FIXME: CE Protocol
EnergomeraCE102 emCE102;

bool _debug = false;
// debug led to VCC:
#define DEBUG_LED_LIGHT LOW
#define DEBUG_LED_DARK HIGH
#define DEBUG_LED_PIN 0 // GPIO0; -1 to disable

WiFiUDP _udp;
int _udpPort = 33333;

unsigned int _counter = 0;// 4 bytes for esp8266
bool _counterChanged = false;
bool _needToReconnect = false;

SemIoTGatewayClient *semiotGtwClient;

#define M_BUF_SIZE 15
int _m_buf[M_BUF_SIZE];
int _m_buf_count=0;

// TODO:
//void _udpDebug(char msg[]) {
//    _udp.beginPacket(softIp, softUdpPort);
//    _udp.write(msg);
//    _udp.endPacket();
//}

void setup() {
    if (_dataSerial) {
        _dataSerial->begin(SERIAL_BAUDRATE);
        // TODO:
    //    if (_debug) {
    //        // debug init
    //    }
        if (DEBUG_LED_PIN>-1) {
            pinMode(DEBUG_LED_PIN, OUTPUT);
        }
        // TODO: custom debug via udp
        semiotGtwClient = new SemIoTGatewayClient(&_udp,_udpPort,NULL,DEBUG_LED_PIN);
        // TODO: remove it, repeating in loop():
        // TODO: EEPROM
        while (WiFi.status() != WL_CONNECTED) {
            connectToWPS(NULL,DEBUG_LED_PIN); // TODO: async via udp SoftAP
        }
        semiotGtwClient->connectToSemIoTGateway();
    }
}

void updateData() {
    emCE102.ReadTariffSum(deviceAddress);
    delay(3000);
    if (_dataSerial->available()) {
        while (_dataSerial->available()!=0) {
            if (_m_buf_count<M_BUF_SIZE) {
                _m_buf[_m_buf_count]=_dataSerial->read();
                _m_buf_count++;
            }
        }
        // TODO: parse to counter _m_buf[_m_buf_count]
        // and change _counter_changed bool
    }
}

void loop() {
    if (_needToReconnect) {
        connectToWPS(NULL,DEBUG_LED_PIN); // TODO: async via udp SoftAP
    }
    updateData();
    semiotGtwClient->sendCounters(_modelWord,&_counter,&_counterChanged,&_needToReconnect);
}
