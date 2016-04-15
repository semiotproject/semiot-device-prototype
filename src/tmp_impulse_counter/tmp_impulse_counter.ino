// ESP-8266 WiFi-module based
// impulse counter
// which saves values value to EEPROM
// and sends current values via UDP
// to automatically recognized local server

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "microcoap.h"

String ap_ssid = "M201";
String ap_password = "Str0ngHack3rPa$$";

String sta_ssid = "ISST";
String sta_password = "Ikcx9Iaau4";

ESP8266WebServer server(80);

// Valtec VLF-R-I: "VLFR"
// Incotex Mercury M201: "M201"
String  _modelWord = "M201";
String  _idWord = "SMT1";
unsigned int _pulseCounter = 0;

WiFiUDP _udp;
char _udpHost[] = "88.201.205.72";
int _udpPort = 49470;

void handleRoot() {
        server.send(200, "text/html", "<h1>You are connected to awesome SemIoT device</h1></br>If you don't, download our mf application somewhere!");
}

void handle(bool set = false) {
    String answer = "{\n";
    answer+="\"answer\": {";
    if (server.hasArg("model"))  {
        if (set) {
            _modelWord=server.arg("model");
        }
        answer+="\"model\": \"" + _modelWord + "\",\n";
    }
    if (server.hasArg("serial"))  {
        if (set) {
            _idWord=server.arg("serial");
        }
        answer+="\"serial\": \"" + _idWord + "\",\n";
    }
    if (server.hasArg("counter"))  {
        if (set) {
            _pulseCounter=server.arg("counter").toInt();
        }
        answer+="\"counter\": \"" + String(_pulseCounter) + "\",\n";
    }
    if (server.hasArg("mac")) {
        answer+="\"mac\": \"" + WiFi.macAddress() + "\",\n";
    }
    if (server.hasArg("sta-ssid"))  {
        answer+="\"sta-ssid\": \"" + sta_ssid + "\",\n";
    }
    if (server.hasArg("ap-ssid"))  {
        answer+="\"ap-ssid\": \"" + ap_ssid + "\",\n";
    }
    if (server.hasArg("wifi-status")) {
        switch (WiFi.status()) {
            case WL_NO_SHIELD:
              answer+="\"wifi-status\": \"WL_NO_SHIELD\",\n";
              break;
            case WL_IDLE_STATUS:
              answer+="\"wifi-status\": \"WL_IDLE_STATUS\",\n";
              break;
            case WL_NO_SSID_AVAIL:
              answer+="\"wifi-status\": \"WL_NO_SSID_AVAIL\",\n";
              break;
            case WL_SCAN_COMPLETED:
              answer+="\"wifi-status\": \"WL_SCAN_COMPLETED\",\n";
              break;
            case WL_CONNECTED:
              answer+="\"wifi-status\": \"WL_CONNECTED\",\n";
              break;
            case WL_CONNECT_FAILED:
              answer+="\"wifi-status\": \"WL_CONNECT_FAILED\",\n";
              break;
            case WL_CONNECTION_LOST:
              answer+="\"wifi-status\": \"WL_CONNECTION_LOST\",\n";
              break;
            case WL_DISCONNECTED:
              answer+="\"wifi-status\": \"WL_DISCONNECTED\",\n";
              break;
            default:
              answer+="\"wifi-status\": \"UNKNOWN\",\n";
              break;
        }
    }
    answer+="\"status\": \"ok\"\n"; // TODO: status field
    answer+="}\n"; // answer
    answer+="}";
    server.send(200, "application/json", answer);
}

void handleGet() {
    handle();
}

void handleSet() {
    handle(true);
}

void handleConnect() {
    WiFi.mode(WIFI_AP_STA); // WIFI_AP, WIFI_STA is ok, WIFI_AP_STA causes problems
    String answer = "{\n";
    answer+="\"answer\": {";
    if (server.hasArg("ssid"))  {
        sta_ssid=server.arg("ssid");
        answer+="\"ssid\": \"" + sta_ssid + "\",\n";
    }
    if (server.hasArg("pass"))  {
        sta_password=server.arg("pass");
        answer+="\"pass\": \"" + sta_password + "\",\n";
    }
    WiFi.begin(sta_ssid.c_str(),sta_password.c_str());
    IPAddress localIP = WiFi.localIP();
    Serial.print("Local IP address: ");
    Serial.println(localIP);
    Serial.println("------------------------");
    answer+="\"status\": \"ok\"\n"; // TODO: status field
    answer+="}\n"; // answer
    answer+="}";
    server.send(200, "application/json", answer);
}

void setup() {
    ESP.eraseConfig();
    delay(1000);
    Serial.begin(115200);
    WiFi.mode(WIFI_AP); // WIFI_AP, WIFI_STA is ok, WIFI_AP_STA causes problems
    WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleRoot); // TODO: hydra-style
    server.on("/get", handleGet);
    server.on("/set", handleSet);
    server.on("/connect", handleConnect);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    /*
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(_udp.beginPacket(_udpHost,_udpPort),DEC);
        _udp.write(_modelWord);
        _udp.write(_idWord);
        _udp.write(_pulseCounter);
        _udp.endPacket();
        _pulseCounter++;
    }
    */
    server.handleClient();
    //delay(3000);
}
