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

void setup() {
    dht22.begin(dht22Pin,DHT22);
    coap.setButton(buttonPin);
    coap.addResource(&temperatureRes);
    coap.addResource(&humidityRes);
    coap.begin(true); // sleepy
}

void loop() {
    updateDHT();
    coap.handleClient();
}
