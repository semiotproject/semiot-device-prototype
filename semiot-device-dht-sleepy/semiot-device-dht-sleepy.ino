#include "DHT.h"
#include "minicoap.h"
#include "temperatureresource.h"
#include "humidityresource.h"

MiniCoAP coap;

int buttonPin = D7;

TemperatureResource temperatureRes;
HumidityResource humidityRes;

DHT dht22;
int dht22Pin = D5;

void updateDHT() {
    temperatureRes.setValue(dht22.readTemperature());
    humidityRes.setValue(dht22.readHumidity());
}

// you don't need it
void additionalHighPin() {
    pinMode(D6,OUTPUT);
    digitalWrite(D6,HIGH);
}

void setup() {
    additionalHighPin(); //FIXME
    dht22.begin(dht22Pin,DHT22);
    coap.setButton(buttonPin);
    coap.addResource(&temperatureRes);
    coap.addResource(&humidityRes);
    coap.begin(false); // sleepy = true
}

void loop() {
    // updateDHT();
    coap.handleClient();
}
