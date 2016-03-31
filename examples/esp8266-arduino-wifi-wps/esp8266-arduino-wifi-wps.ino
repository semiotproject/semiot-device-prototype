#include "WiFiex.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Start WPS connection via your router");
    connectToWPS(); // from WiFiex.h
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WPS");
        delay(1000);
    }
}
