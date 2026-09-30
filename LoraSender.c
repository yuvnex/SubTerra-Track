// LoRaSender.ino — SubterraTrack Worker Device
// Flash to sender Arduino (worker carries this)

#include <SPI.h>
#include <LoRa.h>

#define SS    10
#define RST   9
#define DIO0  2

#define WORKER_ID   "Worker1"   // Change for each worker: Worker2, Worker3...
#define GAS_ANALOG  A0

// Calibrate these after testing with raw reader
#define GAS_SAFE     230
#define GAS_WARNING  380
#define GAS_TOXIC    580

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(GAS_ANALOG, INPUT);

  Serial.println("Warming up MQ-135... 30 seconds");
  delay(30000);
  Serial.println("MQ-135 Ready!");

  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);
  delay(20);
  digitalWrite(RST, HIGH);
  delay(150);

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setTxPower(20);
  LoRa.setSyncWord(0xF3);
  LoRa.enableCrc();

  Serial.println("Sender ready!");
}

void loop() {
  int gasValue = analogRead(GAS_ANALOG);
  String gasStatus = "SAFE";

  if (gasValue >= GAS_TOXIC) {
    gasStatus = "TOXIC";
  } else if (gasValue >= GAS_WARNING) {
    gasStatus = "WARNING";
  } else if (gasValue >= GAS_SAFE) {
    gasStatus = "MILD";
  }

  // Format: Worker1,counter,gasValue,gasStatus
  String message = String(WORKER_ID) + "," +
                   String(counter)   + "," +
                   String(gasValue)  + "," +
                   gasStatus;

  Serial.print("Sending: ");
  Serial.println(message);

  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket(false);

  counter++;
  delay(3000);
}
