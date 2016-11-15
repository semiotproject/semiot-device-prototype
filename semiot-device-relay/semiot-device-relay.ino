#include "minicoap.h"
#include "relayresource.h"

MiniCoAP coap;

int buttonPin = D7;

RelayResource relayRes;

int relayPin = D5;

void setup() {
    relayRes.setPin(relayPin);
    coap.setButton(buttonPin);
    coap.addResource(&relayRes);
    coap.begin();
}

void loop() {
    coap.handleClient();
}
