#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define AS5600_ADDR 0x36

// Direcciones de registros
#define ZPOS_H       0x01
#define MPOS_H       0x03
#define RAW_ANGLE_H  0x0C
#define ANGLE_H      0x0E
#define STATUS       0x0B
#define AGC_REG      0x1A
#define MAGNITUDE_H  0x1B

// Pines de los dos buses I2C
// #define SDA_1 21
// #define SCL_1 22
#define SDA_2 25
#define SCL_2 26

// Pines de la SD
#define SD_CS 5

// Botón para cerrar/crear archivo nuevo
#define BUTTON_PIN 4

File logFile;
int flightNumber = 0;
bool buttonState = false;
bool lastButtonState = false;

// ============================================================
// Funciones de lectura/escritura genéricas (por bus)
// ============================================================

void writeRegister16(TwoWire &bus, uint8_t regH, uint16_t value) {
  bus.beginTransmission(AS5600_ADDR);
  bus.write(regH);
  bus.write((value >> 8) & 0x0F);
  bus.write(value & 0xFF);
  bus.endTransmission();
}

uint16_t readRegister16(TwoWire &bus, uint8_t regH) {
  bus.beginTransmission(AS5600_ADDR);
  bus.write(regH);
  bus.endTransmission(false);
  bus.requestFrom(AS5600_ADDR, (uint8_t)2);
  if (bus.available() < 2) return 0xFFFF;
  uint8_t hi = bus.read();
  uint8_t lo = bus.read();
  return ((uint16_t)hi << 8 | lo) & 0x0FFF;
}

uint8_t readRegister8(TwoWire &bus, uint8_t reg) {
  bus.beginTransmission(AS5600_ADDR);
  bus.write(reg);
  bus.endTransmission(false);
  bus.requestFrom(AS5600_ADDR, (uint8_t)1);
  if (!bus.available()) return 0xFF;
  return bus.read();
}

// ============================================================
// Funciones de alto nivel (por bus)
// ============================================================

uint16_t readRawAngle(TwoWire &bus) { return readRegister16(bus, RAW_ANGLE_H); }
uint16_t readAngle(TwoWire &bus)    { return readRegister16(bus, ANGLE_H); }
uint16_t readMagnitude(TwoWire &bus){ return readRegister16(bus, MAGNITUDE_H); }
uint8_t  readAGC(TwoWire &bus)      { return readRegister8(bus, AGC_REG); }
uint8_t  readStatus(TwoWire &bus)   { return readRegister8(bus, STATUS); }

void configureRange(TwoWire &bus, uint16_t zpos, uint16_t mpos) {
  writeRegister16(bus, ZPOS_H, zpos);
  delay(1);
  writeRegister16(bus, MPOS_H, mpos);
  delay(1);
}

// ============================================================
// Manejo del contador de vuelo (persistente en SD)
// ============================================================

int readFlightCounter() {
  File counterFile = SD.open("/contador.txt");
  if (!counterFile) {
    Serial.println("No se encontró contador.txt, iniciando en 0.");
    return 0; // primer valor
  }
  String value = counterFile.readStringUntil('\n');
  counterFile.close();
  return value.toInt() >= 0 ? value.toInt() : 0;
}

void saveFlightCounter(int num) {
  File counterFile = SD.open("/contador.txt", FILE_WRITE);
  if (!counterFile) {
    Serial.println("⚠️ No se pudo guardar contador.txt");
    return;
  }
  counterFile.seek(0);
  counterFile.println(num);
  counterFile.close();
  Serial.print("Contador actualizado a ");
  Serial.println(num);
}

// ============================================================
// Crear nuevo archivo de vuelo
// ============================================================

void createNewFile() {
  char filename[20];
  sprintf(filename, "/vuelo%d.txt", flightNumber);

  logFile = SD.open(filename, FILE_WRITE);
  if (!logFile) {
    Serial.print("❌ No se pudo crear ");
    Serial.println(filename);
    while (true);
  }

  Serial.print("✅ Nuevo archivo: ");
  Serial.println(filename);

  // Encabezado CSV
  logFile.println("raw1,angle1,deg1,agc1,mag1,MD1,ML1,MH1,raw2,angle2,deg2,agc2,mag2,MD2,ML2,MH2");
  logFile.flush();
}

// ============================================================
// Setup principal
// ============================================================

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);

  // Inicializar SD
  Serial.println("Iniciando tarjeta SD...");
  if (!SD.begin(SD_CS)) {
    Serial.println("❌ Error al iniciar SD");
    while (true);
  }
  Serial.println("✅ SD lista.");

  // Leer número de vuelo
  flightNumber = readFlightCounter() + 1;
  createNewFile();

  // Inicializar buses I2C
  // Wire.begin(SDA_1, SCL_1);
  // Wire.setClock(400000);
  Wire1.begin(SDA_2, SCL_2);
  Wire1.setClock(400000);

  delay(100);
  // configureRange(Wire, 0, 4096);
  configureRange(Wire1, 0, 4096);

  Serial.println("AS5600 dual I2C + SD iniciado.");
}

// ============================================================
// Loop principal
// ============================================================

void loop() {
  // Botón para cambiar de archivo
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState && !lastButtonState) {
    Serial.println("🔹 Botón presionado: nuevo archivo...");
    if (logFile) {
      logFile.println("Cerrando archivo...");
      logFile.close();
      Serial.println("Archivo cerrado.");
    }
    flightNumber++;
    saveFlightCounter(flightNumber);
    delay(300); // anti-rebote
    createNewFile();
  }
  lastButtonState = buttonState;

  // === Lecturas de ambos sensores ===
  // uint16_t raw1 = readRawAngle(Wire);
  // uint16_t angle1 = readAngle(Wire);
  // uint8_t status1 = readStatus(Wire);
  // uint8_t agc1 = readAGC(Wire);
  // uint16_t mag1 = readMagnitude(Wire);
  // bool MH1 = status1 & 0x20;
  // bool ML1 = status1 & 0x10;
  // bool MD1 = status1 & 0x08;

  uint16_t raw2 = readRawAngle(Wire1);
  uint16_t angle2 = readAngle(Wire1);
  uint8_t status2 = readStatus(Wire1);
  uint8_t agc2 = readAGC(Wire1);
  uint16_t mag2 = readMagnitude(Wire1);
  bool MH2 = status2 & 0x20;
  bool ML2 = status2 & 0x10;
  bool MD2 = status2 & 0x08;

  // float deg1 = angle1 * 360.0 / 4096.0;
  float deg2 = angle2 * 360.0 / 4096.0;

  // Escribir en SD (una línea CSV)
  if (logFile) {
    // logFile.print(raw1); logFile.print(",");
    // logFile.print(angle1); logFile.print(",");
    // logFile.print(deg1, 2); logFile.print(",");
    // logFile.print(agc1); logFile.print(",");
    // logFile.print(mag1); logFile.print(",");
    // logFile.print(MD1); logFile.print(",");
    // logFile.print(ML1); logFile.print(",");
    // logFile.print(MH1); logFile.print(",");

    logFile.print(raw2); logFile.print(",");
    logFile.print(angle2); logFile.print(",");
    logFile.print(deg2, 2); logFile.print(",");
    logFile.print(agc2); logFile.print(",");
    logFile.print(mag2); logFile.print(",");
    logFile.print(MD2); logFile.print(",");
    logFile.print(ML2); logFile.print(",");
    logFile.println(MH2);
    logFile.flush(); // asegura grabación
  }

  // También imprimir por Serial (opcional)
  // Serial.print(raw1); Serial.print(",");
  // Serial.print(angle1); Serial.print(",");
  // Serial.print(deg1, 2); Serial.print(",");
  // Serial.print(agc1); Serial.print(",");
  // Serial.print(mag1); Serial.print(",");
  // Serial.print(MD1); Serial.print(",");
  // Serial.print(ML1); Serial.print(",");
  // Serial.print(MH1); Serial.print(",");

  Serial.print(raw2); Serial.print(",");
  Serial.print(angle2); Serial.print(",");
  Serial.print(deg2, 2); Serial.print(",");
  Serial.print(agc2); Serial.print(",");
  Serial.print(mag2); Serial.print(",");
  Serial.print(MD2); Serial.print(",");
  Serial.print(ML2); Serial.print(",");
  Serial.println(MH2);

  delay(200);
}
