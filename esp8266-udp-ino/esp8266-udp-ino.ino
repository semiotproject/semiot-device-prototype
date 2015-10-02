// https://github.com/esp8266/Arduino
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Search for your device by MAC adddress

// DHT-sensor-library -- Arduino library for the DHT11/DHT22 temperature and humidity sensors
// https://github.com/adafruit/DHT-sensor-library
#include "DHT.h"

#define DHTPIN 2 // what pin we're connected to DHT
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

byte mac[6];
char d;
#define PORT 32001
char h;
char t;

IPAddress ip; // = WiFi.localIP();

WiFiUDP g_udp;

#define SSID "ISST"
#define PASSWORD "Ikcx9Iaau4"

#define TIME_PERIOD 600000000 // us -- 10 min

void setup() {
    Serial.begin(115200);
    dht.begin();
    WiFi.mode(WIFI_AP);
    WiFi.macAddress(mac);
    while ( WiFi.status() != WL_CONNECTED) {
	Serial.print("Attempting to connect to WPA SSID: ");
	Serial.println(SSID);
	// Connect to WPA/WPA2 network:    
	WiFi.begin(SSID, PASSWORD);
	// wait 10 seconds for connection:
	delay(10000);
    }
    Serial.println("\nStarting connection to server...");
    g_udp.begin(PORT);
    ip = WiFi.localIP();
    ip[3] = 255; // broadcast
}

void loop() {
    h = dht.readHumidity();
    t = dht.readTemperature();
    d = isnan(h) || isnan(t);
    
    g_udp.beginPacket(ip, PORT);
    // FIXME: Magic numbers:
    g_udp.write(mac, size_t(6));
    g_udp.write(&d, size_t(1));
    g_udp.write(&h, size_t(4));
    g_udp.write(&t, size_t(4));
    g_udp.endPacket();
    ESP.deepSleep(TIME_PERIOD, WAKE_RF_DISABLED);
}