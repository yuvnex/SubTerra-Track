// LoRaReceiver.ino — SubterraTrack Base Station
// Flash to receiver Arduino (connected to laptop via USB)
// Output format: Worker1,-72,350,WARNING  (parsed by index.html Web Serial)

#include <SPI.h>
#include <LoRa.h>

#define SS    10
#define RST   9
#define DIO0  2

unsigned long lastReceived = 0;
unsigned long lastScanned  = 0;

void initLoRa() {
  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);
  delay(20);
  digitalWrite(RST, HIGH);
  delay(150);

  LoRa.setPins(SS, RST, DIO0);

  while (!LoRa.begin(433E6)) {
    Serial.println("Retrying...");
    delay(500);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0xF3);
  LoRa.enableCrc();

  Serial.println("READY");
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  initLoRa();
}

void loop() {
  if (millis() - lastScanned > 3000) {
    lastScanned = millis();
  }

  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    String received = "";

    for (int i = 0; i < packetSize; i++) {
      if (LoRa.available()) {
        char c = (char)LoRa.read();
        if (c >= 32 && c <= 126) received += c;
      }
    }
    while (LoRa.available()) LoRa.read();
    received.trim();

    int rssi = LoRa.packetRssi();

    // Debug: print raw packet
    Serial.println("DEBUG_PKT:" + received + "|RSSI:" + String(rssi));

    // Filter invalid RSSI
    if (rssi < -30 && rssi > -120) {

      // Parse: Worker1,counter,gasValue,gasStatus
      int comma1 = received.indexOf(',');
      int comma2 = received.indexOf(',', comma1 + 1);
      int comma3 = received.indexOf(',', comma2 + 1);

      if (comma1 > 0 && comma2 > 0 && comma3 > 0) {
        String workerId  = received.substring(0, comma1);
        String gasValue  = received.substring(comma2 + 1, comma3);
        String gasStatus = received.substring(comma3 + 1);

        // Output for Web Serial dashboard: Worker1,-72,350,WARNING
        String output = workerId + "," + String(rssi) + "," + gasValue + "," + gasStatus;
        Serial.println(output);
        Serial.flush();

        lastReceived = millis();
      } else {
        Serial.println("DEBUG_PARSE_FAIL:" + received + "|c1:" + String(comma1) + "|c2:" + String(comma2) + "|c3:" + String(comma3));
      }
    }
  }

  if (millis() - lastReceived > 30000 && lastReceived != 0) {
    Serial.println("Reinitializing...");
    LoRa.end();
    delay(500);
    initLoRa();
    lastReceived = millis();
  }
}
