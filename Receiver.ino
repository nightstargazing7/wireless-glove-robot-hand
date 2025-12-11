#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_PWMServoDriver.h>

RF24 radio(9, 10);
const byte address[6] = "node1";

struct FlexData {
  int finger[5];
};
FlexData data;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

#define CH_THUMB 0
#define CH_INDEX 1
#define CH_MIDDLE 2
#define CH_RING 3
#define CH_PINKY 4

int SERVOMIN = 150;
int SERVOMAX = 600;

int controlMode = 0;

int angleToPulse(int angle){
  angle = constrain(angle, 0, 150);
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

void setServo(uint8_t ch, int angle){
  pwm.setPWM(ch, 0, angleToPulse(angle));
}

void goAll(int angle){
  angle = constrain(angle, 0, 150);
  setServo(CH_THUMB, angle);
  setServo(CH_INDEX, angle);
  setServo(CH_MIDDLE, angle);
  setServo(CH_RING, angle);
  setServo(CH_PINKY, angle);
}

void setServoByIndex(int idx, int angle){
  angle = constrain(angle, 0, 150);
  if (idx == 0) setServo(CH_THUMB, angle);
  else if (idx == 1) setServo(CH_INDEX, angle);
  else if (idx == 2) setServo(CH_MIDDLE, angle);
  else if (idx == 3) setServo(CH_RING, angle);
  else if (idx == 4) setServo(CH_PINKY, angle);
}

void handleSerialCommands() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;
  String lower = line;
  lower.toLowerCase();

  if (lower == "t") {
    controlMode = 1;
    goAll(0);
    return;
  }

  if (lower == "g") {
    controlMode = 2;
    goAll(0);
    return;
  }

  if (lower == "r") {
    controlMode = 0;
    return;
  }

  if (controlMode == 1) {
    int ang = line.toInt();
    if (ang >= 0 && ang <= 150) goAll(ang);
    return;
  }

  if (controlMode == 2) {
    int spacePos = line.indexOf(' ');
    if (spacePos <= 0) return;
    int idx = line.substring(0, spacePos).toInt();
    int ang = line.substring(spacePos + 1).toInt();
    if (idx >= 0 && idx <= 4 && ang >= 0 && ang <= 150) setServoByIndex(idx, ang);
    return;
  }
}

void setup(){
  Serial.begin(9600);
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(50);
  goAll(90);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setChannel(76);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
}

void loop() {
  handleSerialCommands();
  if (controlMode != 0) return;

  if (radio.available()) {
    radio.read(&data, sizeof(data));

    int thumbPos  = map(data.finger[0], 500, 520, 0, 150);
    int indexPos  = map(data.finger[1], 400, 475, 0, 150);
    int middlePos = map(data.finger[2], 185, 275, 0, 150);
    int ringPos   = middlePos;
    int pinkyPos  = map(data.finger[4], 420, 550, 0, 150);

    thumbPos  = constrain(thumbPos, 0, 150);
    indexPos  = constrain(indexPos, 0, 150);
    middlePos = constrain(middlePos, 0, 150);
    ringPos   = constrain(ringPos, 0, 150);
    pinkyPos  = constrain(pinkyPos, 0, 150);

    setServo(CH_THUMB, thumbPos);
    setServo(CH_INDEX, indexPos);
    setServo(CH_MIDDLE, middlePos);
    setServo(CH_RING, ringPos);
    setServo(CH_PINKY, pinkyPos);
  }
}
