#include <Wire.h>
#define AS5600_ADDR 0x36
#define RAW_ANGLE_H 0x0C
#define RAW_ANGLE_L 0x0D
#define ANGLE_H 0x0E
#define ANGLE_L 0x0F

// Pines I2C por defecto en muchas placas ESP32: SDA=21, SCL=22
#define I2C_SDA 21
#define I2C_SCL 22

void setup() {
  Serial.begin(115200);
  // inicializa I2C en los pines elegidos y pone frecuencia a 400kHz
  Wire.begin(I2C_SDA, I2C_SCL); 
  Wire.setClock(400000);
  delay(100);
}

uint16_t readRawAngle() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(RAW_ANGLE_H);
  Wire.endTransmission(false); // repeated start
  Wire.requestFrom(AS5600_ADDR, (uint8_t)2);
  if (Wire.available() < 2) return 0xFFFF; // error
  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  return ((uint16_t)hi << 8 | lo) & 0x0FFF; // 12 bits
}

uint16_t readAngle() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(ANGLE_H);
  Wire.endTransmission(false); // repeated start
  Wire.requestFrom(AS5600_ADDR, (uint8_t)2);
  if (Wire.available() < 2) return 0xFFFF; // error
  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  return ((uint16_t)hi << 8 | lo) & 0x0FFF; // 12 bits
}

void loop() {
  uint16_t raw = readRawAngle();
  uint16_t angle = readAngle();

  // if (raw == 0xFFFF || angle == 0xFFFF) {
  if (raw == 0xFFFF){
    Serial.println("I2C read error");
  } else {
    float degRaw   = raw   * 360.0 / 4096.0;
    float degAngle = angle * 360.0 / 4096.0;

    Serial.print("RAW counts: "); Serial.print(raw);
    Serial.print("  RAW deg: "); Serial.print(degRaw, 3);

    Serial.print("  |  ANGLE counts: "); Serial.print(angle);
    Serial.print("  ANGLE deg: "); Serial.println(degAngle, 3);
  }
  delay(50);
}