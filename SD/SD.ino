#include <SPI.h>
#include <SD.h>

#define SD_CS 5
#define BUTTON_PIN 4

File logFile;
int flightNumber = 1;
bool buttonState = false;
bool lastButtonState = false;

// ----------------------------------------------------
// Leer contador desde SD
// ----------------------------------------------------
int readFlightCounter() {
  File counterFile = SD.open("/contador.txt");
  if (!counterFile) {
    Serial.println("No se encontró contador.txt, creando nuevo (1).");
    return 0; // valor inicial
  }
  String value = counterFile.readStringUntil('\n');
  counterFile.close();
  return value.toInt() >= 0 ? value.toInt() : 0;
}

// ----------------------------------------------------
// Guardar contador actualizado en SD
// ----------------------------------------------------
void saveFlightCounter(int num) {
  File counterFile = SD.open("/contador.txt", FILE_WRITE);
  if (!counterFile) {
    Serial.println("⚠️ No se pudo escribir contador.txt");
    return;
  }
  counterFile.seek(0);      // sobreescribe desde inicio
  counterFile.println(num); // guarda nuevo valor
  counterFile.close();
  Serial.print("Contador actualizado a ");
  Serial.println(num);
}

// ----------------------------------------------------
// Crear nuevo archivo con nombre vuelo#.txt
// ----------------------------------------------------
void createNewFile() {
  char filename[20];
  sprintf(filename, "/vuelo%d.txt", flightNumber);

  logFile = SD.open(filename, FILE_WRITE);
  if (!logFile) {
    Serial.print("❌ No se pudo crear ");
    Serial.println(filename);
    while (true);
  }

  Serial.print("✅ Nuevo archivo creado: ");
  Serial.println(filename);

  logFile.println("Archivo iniciado");
  logFile.flush();
}

// ----------------------------------------------------
// Setup principal
// ----------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT_PULLDOWN);

  Serial.println("Iniciando tarjeta SD...");
  if (!SD.begin(SD_CS)) {
    Serial.println("❌ Error al iniciar SD");
    while (true);
  }
  Serial.println("✅ SD inicializada");

  // Leer número de vuelo guardado
  flightNumber = readFlightCounter() + 1;
  createNewFile();
}

// ----------------------------------------------------
// Loop principal
// ----------------------------------------------------
void loop() {
  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState && !lastButtonState) {
    Serial.println("🔹 Botón presionado: cambiando archivo...");
    if (logFile) {
      logFile.println("Cerrando archivo...");
      logFile.close();
      Serial.println("Archivo cerrado.");
    }

    // Incrementar y guardar contador
    flightNumber++;
    saveFlightCounter(flightNumber);

    delay(300); // anti-rebote
    createNewFile();
  }
  lastButtonState = buttonState;

  // Escribir continuamente
  if (logFile) {
    logFile.println("Hola mundo");
    logFile.flush();
  }

  delay(500);
}
