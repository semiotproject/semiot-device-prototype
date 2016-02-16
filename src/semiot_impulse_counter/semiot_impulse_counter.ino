#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

// FORMAT:
// "WORD" (4B)
// imp_counter (2B)
// high_counter (4B)
// MAC (6B)
// == 16 BYTES
// TODO: checksum?

// Valtec VLF-R-I: "VLFR"
// Incotex Mercury M201: "M201"
#define MODEL_WORD "VLFR"

#define PULSE_PIN 2 // GPIO2
// debug led to VCC:
#define DEBUG_LED_PIN 0 // GPIO0
#define DEBUG_LED_LIGHT LOW
#define DEBUG_LED_DARK HIGH

bool _debug = false;
bool _debug_led = true;

#define MAX_COUNTER_LOW_NUMBER 10
#define SERIAL_BAUDRATE 115200

uint16_t imp_counter = 0; // 2 bytes
unsigned int high_counter = 0; // 4 bytes for esp8266
bool counter_changed = false;
bool need_to_reconnect = false;

WiFiUDP _udp;
IPAddress ip;
byte mac[6];
const int udp_port = 33333;
#define UDP_GTW_OK_SIZE 5
#define UDP_GTW_OK "GTWOK"
#define UDP_GTW_PING "GTW"
char gtw_ok_buffer[UDP_GTW_OK_SIZE];
char gtw_ip;

void read_counters_from_eeprom() {
    //EEPROM.begin(16);
    // TODO: "M201 and newline"
    // NOTE: platform specific:
    // FIXME: magic numbers from packet format:
    int _imp_counter = ((unsigned char)(EEPROM.read(4)) << 8) + (unsigned char)EEPROM.read(5);
    if (_imp_counter<MAX_COUNTER_LOW_NUMBER) {
        imp_counter=_imp_counter;
        if (_debug) {
            Serial.print("EEPROM _imp_counter= ");
            Serial.println(_imp_counter,DEC);
        }
        int _high_counter = ((unsigned char)(EEPROM.read(6)) << 24) + ((unsigned char)(EEPROM.read(7)) << 16) + ((unsigned char)(EEPROM.read(8)) << 8) + (unsigned char)EEPROM.read(9);
        if (_debug) {
            Serial.print("EEPROM _high_counter= ");
            Serial.println(_high_counter,DEC);
        }
        high_counter=_high_counter;
    }
    //EEPROM.end();
}

void write_counters_to_eeprom() {
    // NOTE: platform specific:
    // TODO: "M201 and newline"
    // FIXME: magic numbers from packet format:
    //EEPROM.begin(16);
    EEPROM.write(4,(imp_counter >> 8) & 0xFF);
    EEPROM.write(5,(imp_counter >> 0) & 0xFF);

    EEPROM.write(6,(high_counter >> 24) & 0xFF);
    EEPROM.write(7,(high_counter >> 16) & 0xFF);
    EEPROM.write(8,(high_counter >> 8) & 0xFF);
    EEPROM.write(9,(high_counter >> 0) & 0xFF);
    EEPROM.commit();
    //EEPROM.end();
}

void gtw_search() {
    ip = (~WiFi.subnetMask()) | WiFi.gatewayIP();
    gtw_ip=0;
    while (gtw_ip == 0) {
        if (_debug) {
            Serial.println("sending broadcast:");
            Serial.println(ip[0],DEC);
            Serial.println(ip[1],DEC);
            Serial.println(ip[2],DEC);
            Serial.println(ip[3],DEC);
        }
        if (_udp.beginPacket(ip, udp_port)) {
            _udp.write(UDP_GTW_PING);
            _udp.endPacket();
            if (_debug) {
                Serial.println("...");
            }
            delay(1500);
            int sz = _udp.parsePacket();
            if (_debug) {
                Serial.print("sz=");
                Serial.println(sz,DEC);
            }
            if (sz==UDP_GTW_OK_SIZE) {
                _udp.read(gtw_ok_buffer,UDP_GTW_OK_SIZE);
                if (memcmp(gtw_ok_buffer, UDP_GTW_OK, UDP_GTW_OK_SIZE) == 0) {
                    if (_debug) {
                        Serial.println("found semiot-gateway!");
                    }
                    ip=_udp.remoteIP();
                    break;
                }
            }
        }
    }
}

// WPS:
void reconnect_to_wlan() {
    while (WiFi.status() != WL_CONNECTED) {
        if (_debug_led) {
            digitalWrite(DEBUG_LED_PIN, DEBUG_LED_LIGHT);
        }
        // WiFi.begin("",""); // decided to get rid of this
        if (_debug_led) {
            digitalWrite(DEBUG_LED_PIN, DEBUG_LED_DARK);
        }
        // Long delay required especially soon after power on.
        delay(4000);
        // Check if WiFi is already connected and if not, begin the WPS process.
        if (WiFi.status() != WL_CONNECTED) {
            if (_debug) {
                Serial.println("\nAttempting connection ...");
            }
            WiFi.beginWPSConfig();
            // Another long delay required.
            delay(3000);
            if (WiFi.status() == WL_CONNECTED) {
                if (_debug) {
                    Serial.println("Connected!");
                    Serial.println(WiFi.localIP());
                    Serial.println(WiFi.SSID());
                    Serial.println(WiFi.macAddress());
                }
            }
            else {
                if (_debug) {
                    Serial.println("Connection failed!");
                }
            }
        }
        else {
            if (_debug) {
                Serial.println("\nConnection already established.");
            }
        }
    }
    WiFi.macAddress(mac);
    _udp.begin(udp_port);
    gtw_search();
    if (_debug_led) {
        digitalWrite(DEBUG_LED_PIN, DEBUG_LED_LIGHT);
    }
}

void blink() {
    imp_counter++;
    counter_changed = true;
}

void setup() {
    if (_debug) {
        Serial.begin(SERIAL_BAUDRATE);
    }
    EEPROM.begin(11);
    read_counters_from_eeprom();
    pinMode(PULSE_PIN, INPUT);
    digitalWrite(PULSE_PIN,LOW);
    if (_debug_led) {
        pinMode(DEBUG_LED_PIN, OUTPUT);
        digitalWrite(DEBUG_LED_PIN, DEBUG_LED_LIGHT);
    }
    attachInterrupt(digitalPinToInterrupt(PULSE_PIN), blink, RISING);
    reconnect_to_wlan();
    if (_debug) {
        Serial.println("Setup completed, waiting for rising input");
    }
}

void loop() {
    if (need_to_reconnect==true) {
        need_to_reconnect = false;
        reconnect_to_wlan();
    }
    if (counter_changed==true) {
        counter_changed=false;
        if (imp_counter==MAX_COUNTER_LOW_NUMBER) {
            imp_counter=0;
            high_counter++;
        }
        write_counters_to_eeprom();
        // send some data
        if (WiFi.status() == WL_CONNECTED) {
            if (!_udp.beginPacket(ip, udp_port)) {
                need_to_reconnect==true;
            }
            _udp.write(MODEL_WORD);
            _udp.write((imp_counter >> 8) & 0xFF);
            _udp.write((imp_counter >> 0) & 0xFF);

            _udp.write((high_counter >> 24) & 0xFF);
            _udp.write((high_counter >> 16) & 0xFF);
            _udp.write((high_counter >> 8) & 0xFF);
            _udp.write((high_counter >> 0) & 0xFF);
            _udp.write(mac[0]);
            _udp.write(mac[1]);
            _udp.write(mac[2]);
            _udp.write(mac[3]);
            _udp.write(mac[4]);
            _udp.write(mac[5]);
            if (_udp.endPacket()) {
                need_to_reconnect==true;
            }
            if (_debug) {
                Serial.print("counter = ");
                Serial.println(imp_counter,DEC);
                Serial.print("high counter = ");
                Serial.println(high_counter,DEC);
            }
        }
        else {
            need_to_reconnect==true;
        }
    }
}
