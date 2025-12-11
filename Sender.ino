#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);
const byte address[6] = "node1";

struct FlexData {
  int finger[5];
};
FlexData data;

int smooth(int pin) {
  long s = 0;
  for (int i = 0; i < 10; i++) s += analogRead(pin);
  return s / 10;
}

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setChannel(76);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
}

void loop() {
  data.finger[0] = smooth(A0);
  data.finger[1] = smooth(A1);
  data.finger[2] = smooth(A2);
  data.finger[3] = smooth(A3);
  data.finger[4] = smooth(A4);

  radio.write(&data, sizeof(data));

  delay(15);
}
