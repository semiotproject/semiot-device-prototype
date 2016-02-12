#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

#define PULSE_PIN 2 // GPIO2
// debug led to VCC:
#define DEBUG_LED_PIN 0 // GPIO0
#define DEBUG_LED_LIGHT LOW
#define DEBUG_LED_DARK HIGH

bool _debug = false;
bool _debug_led = true;

// Mercury-201 energy counsumption meter based format:
// 3200 impulses ~= 1 kW*h
// FIXME: universal counter format
#define KWH_IMPULSES_NUMBER 3200
#define SERIAL_BAUDRATE 115200

uint16_t imp_counter = 0; // 2 bytes
unsigned int kWh_counter = 0; // 4 bytes for esp8266
bool counter_changed = false;
bool need_to_reconnect = false;

WiFiUDP _udp;
IPAddress ip;
const int udp_port = 55555;
#define UDP_GTW_OK_SIZE 5
#define UDP_GTW_OK "GTWOK"
#define UDP_GTW_PING "GTW"
char gtw_ok_buffer[UDP_GTW_OK_SIZE];
char gtw_ip;

// FORMAT:
// "M201" (4B)
// imp_counter (2B)
// KWh_counter (4B)
// newline (1B)
// == 10 BYTES
// TODO: checksum?
void read_counters_from_eeprom() {
    //EEPROM.begin(11);
    // TODO: "M201 and newline"
    // NOTE: platform specific:
    // FIXME: magic numbers from packet format:
    int _imp_counter = ((unsigned char)(EEPROM.read(4)) << 8) + (unsigned char)EEPROM.read(5);
    if (_imp_counter<KWH_IMPULSES_NUMBER) {
        imp_counter=_imp_counter;
        if (_debug) {
            Serial.print("EEPROM _imp_counter= ");
            Serial.println(_imp_counter,DEC);
        }
        int _kWh_counter = ((unsigned char)(EEPROM.read(6)) << 24) + ((unsigned char)(EEPROM.read(7)) << 16) + ((unsigned char)(EEPROM.read(8)) << 8) + (unsigned char)EEPROM.read(9);
        if (_debug) {
            Serial.print("EEPROM _kWh_counter= ");
            Serial.println(_kWh_counter,DEC);
        }
        kWh_counter=_kWh_counter;
    }
    //EEPROM.end();
}

void write_counters_to_eeprom() {
    // NOTE: platform specific:
    // TODO: "M201 and newline"
    // FIXME: magic numbers from packet format:
    //EEPROM.begin(11);
    EEPROM.write(4,(imp_counter >> 8) & 0xFF);
    EEPROM.write(5,(imp_counter >> 0) & 0xFF);

    EEPROM.write(6,(kWh_counter >> 24) & 0xFF);
    EEPROM.write(7,(kWh_counter >> 16) & 0xFF);
    EEPROM.write(8,(kWh_counter >> 8) & 0xFF);
    EEPROM.write(9,(kWh_counter >> 0) & 0xFF);
    EEPROM.commit();
    //EEPROM.end();
}

void gtw_search() {
    ip = WiFi.localIP();
    gtw_ip=0;
    while (gtw_ip == 0) {
        for (int i=1;i<255;i++) { //TODO: do not send to yourself
            if (_debug) {
                Serial.println(i,DEC);
            }
            ip[3] = i; // FIXME: recognize local gateway IP
            if (_udp.beginPacket(ip, udp_port)) {
                _udp.write(UDP_GTW_PING);
                _udp.endPacket();
                delay(100);
                int sz = _udp.parsePacket();
                if (sz==UDP_GTW_OK_SIZE) {
                    _udp.read(gtw_ok_buffer,UDP_GTW_OK_SIZE);
                    if (strcmp(gtw_ok_buffer, UDP_GTW_OK) == 0) {
                        gtw_ip=_udp.remoteIP()[3];
                        break;
                    }
                }
            }
        }
    }
    ip[3]=gtw_ip;
    if (_debug) {
        Serial.print("found semiot-gateway, last ipv4 octet: ");
        Serial.println(gtw_ip, DEC);
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
        if (imp_counter==KWH_IMPULSES_NUMBER) {
            imp_counter=0;
            kWh_counter++;
        }
        write_counters_to_eeprom();
        // send some data
        if (WiFi.status() == WL_CONNECTED) {
            if (!_udp.beginPacket(ip, udp_port)) {
                need_to_reconnect==true;
            }
            _udp.write("M201");
            _udp.write((imp_counter >> 8) & 0xFF);
            _udp.write((imp_counter >> 0) & 0xFF);

            _udp.write((kWh_counter >> 24) & 0xFF);
            _udp.write((kWh_counter >> 16) & 0xFF);
            _udp.write((kWh_counter >> 8) & 0xFF);
            _udp.write((kWh_counter >> 0) & 0xFF);
            _udp.write("\n");
            if (_udp.endPacket()) {
                need_to_reconnect==true;
            }
            if (_debug) {
                Serial.print("counter = ");
                Serial.println(imp_counter,DEC);
                Serial.print("W*h = ");
                Serial.println(imp_counter*0.3125,DEC); // 1000/3200: 3200 impulses ~= 1 kW*h
            }
        }
        else {
            need_to_reconnect==true;
        }
    }
}
