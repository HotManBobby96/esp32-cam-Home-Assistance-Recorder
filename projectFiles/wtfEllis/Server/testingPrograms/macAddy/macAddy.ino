#include "WiFi.h"

void setup() {
  Serial.begin(9600);
  delay(1000);

  // Get the MAC address
  String macAddress = WiFi.macAddress();
  Serial.println("MAC Address: " + macAddress);
}

void loop() {
  // fc:e8:c0:a0:8a:ac
}
