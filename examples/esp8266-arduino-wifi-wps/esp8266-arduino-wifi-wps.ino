#include "WiFiex.h"

void setup() {
    connectToWPS();
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Still connected");
    }
    delay(1000);
}
