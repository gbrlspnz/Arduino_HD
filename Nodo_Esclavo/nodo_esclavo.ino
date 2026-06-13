#include <LiquidCrystal.h>

// --- PINES ---
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // RS, E, D4, D5, D6, D7
const int pinTemp = A0;

// --- ESTADO DEL SISTEMA ---
float temperatura      = 0.0;
bool  modoFull         = false;   // true cuando el Maestro activó modo manual
bool  sinSenal         = false;   // true cuando no llegan peticiones del Maestro

// --- CONTROL DE TIEMPO ---
unsigned long ultimaActualizacionLCD = 0;
unsigned long ultimaPeticionRecibida = 0;  // para detectar pérdida de señal
const unsigned long TIMEOUT_SENAL    = 5000; // 5 s sin petición → "Sin señal"

// --- BUFFER DE RECEPCIÓN UART (usado por serialEvent) ---
byte bufferRX[8];
byte indexRX = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  ultimaPeticionRecibida = millis();
}

void loop() {
  leerTemperatura();
  verificarSenal();
  actualizarLCD();
}

void serialEvent() {
  while (Serial.available()) {
    byte b = Serial.read();

    // Sincronización: si el buffer está vacío, solo aceptar SOF
    if (indexRX == 0 && b != 0xAA) continue;

    bufferRX[indexRX++] = b;

    // Trama mínima: SOF + ID + LEN + CHK = 4 bytes (LEN=0)
    // Esperamos al menos 4 bytes para evaluar
    if (indexRX < 4) continue;

    byte id  = bufferRX[1];
    byte len = bufferRX[2];
    byte tramaTotalEsperada = 4 + len; // SOF + ID + LEN + DATA(len) + CHK

    // Todavía no llegó la trama completa
    if (indexRX < tramaTotalEsperada) continue;

    // Trama completa recibida — validar checksum
    byte chkRecibido = bufferRX[tramaTotalEsperada - 1];
    byte chkCalculado = id ^ len;
    for (byte i = 0; i < len; i++) {
      chkCalculado ^= bufferRX[3 + i];
    }

    if (chkCalculado == chkRecibido) {
      procesarMensaje(id, len, &bufferRX[3]);
    }
    // Si checksum falla, se descarta silenciosamente y se resetea

    // Resetear buffer para la siguiente trama
    indexRX = 0;
  }
}

void procesarMensaje(byte id, byte len, byte* data) {
  ultimaPeticionRecibida = millis(); // Maestro está vivo
  sinSenal = false;

  switch (id) {

    case 0x01: // Solicitud de temperatura
      enviarTemperatura();
      break;

    case 0x03: // Comando de modo desde el Maestro
      if (len >= 1) {
        modoFull = (data[0] == 0x01);
      }
      enviarACK();
      break;

    default:
      break; // Tipo de mensaje desconocido, ignorar
  }
}

void leerTemperatura() {
  int lectura  = analogRead(pinTemp);
  float voltaje = lectura * (5.0 / 1024.0);
  temperatura   = (voltaje - 0.5) * 100.0;
}

void enviarTemperatura() {
  int tempX10  = (int)(temperatura * 10.0);
  byte high    = (tempX10 >> 8) & 0xFF;
  byte low     = tempX10 & 0xFF;
  byte chk     = 0x02 ^ 0x02 ^ high ^ low;
  byte trama[] = {0xAA, 0x02, 0x02, high, low, chk};
  Serial.write(trama, 6);
}

void enviarACK() {
  byte chk     = 0x04 ^ 0x00;
  byte trama[] = {0xAA, 0x04, 0x00, chk};
  Serial.write(trama, 4);
}

void verificarSenal() {
  if (millis() - ultimaPeticionRecibida > TIMEOUT_SENAL) {
    sinSenal = true;
  }
}

void actualizarLCD() {
  if (millis() - ultimaActualizacionLCD < 500) return;
  ultimaActualizacionLCD = millis();

  lcd.clear();

  if (sinSenal) {
    lcd.setCursor(0, 0);
    lcd.print("!! Sin senal !!");
    lcd.setCursor(0, 1);
    lcd.print("Verificar cable");
    return;
  }

  // Línea 1: temperatura
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperatura, 1); // 1 decimal
  lcd.print(" C");

  // Línea 2: modo activo
  lcd.setCursor(0, 1);
  if (modoFull) {
    lcd.print(">> MODO FULL <<");
  } else {
    lcd.print("Modo: Automatico");
  }
}
