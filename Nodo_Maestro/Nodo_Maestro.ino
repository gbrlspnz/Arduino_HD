// ============================================================
//  ARDUINO MAESTRO — Control de motor con TB6612FNG
//  Adaptado desde L293D → TB6612FNG
//  Cambios: pin STBY agregado (D10), activado en setup()
//  El resto de la lógica es idéntica al original
// ============================================================

// --- PINES LED RGB (cátodo común) ---
const int pinLedRojo  = 5;   // Pin D5 → resistencia 220Ω → pata R del RGB
const int pinLedVerde = 6;   // Pin D6 → resistencia 220Ω → pata G del RGB
const int pinLedAzul  = 7;   // Pin D7 → resistencia 220Ω → pata B del RGB

// --- PINES MOTOR (TB6612FNG) ---
const int motorPWM  = 11;  // D11 → PWMA  del TB6612FNG
const int motorIN1  =  9;  // D9  → AIN1  del TB6612FNG
const int motorIN2  =  8;  // D8  → AIN2  del TB6612FNG
const int motorSTBY = 10;  // D10 → STBY  del TB6612FNG  ← NUEVO

// --- OTROS PINES ---
const int pinBoton  = 2;

// --- PARÁMETROS DEL SISTEMA ---
const float UMBRAL_TEMP            = 20.0; // °C — por encima activa motor en modo auto
const int   VELOCIDAD_FULL         = 255;  // PWM máximo (modo full / alerta)
const int   VELOCIDAD_NORMAL       = 0;    // Motor apagado en condición normal
const unsigned long INTERVALO_PETICION = 1500; // ms entre solicitudes de temperatura
const unsigned long TIMEOUT_RESPUESTA  = 2000; // ms máximo para esperar respuesta

// --- ESTADO DEL SISTEMA ---
float temperatura        = 0.0;
bool  modoManual         = false;
bool  errorComunicacion  = false;
bool  esperandoRespuesta = false;

// --- MEDICIÓN DE EFICIENCIA (para análisis polling vs ISR) ---
unsigned long tiempoEnvio       = 0; // micros() al enviar petición
unsigned long latenciaRespuesta = 0; // micros entre envío y recepción

// --- CONTROL DE TIEMPO ---
unsigned long ultimaPeticion = 0;
unsigned long tiempoEspera   = 0;

// --- ANTIRREBOTE DEL BOTÓN ---
bool estadoBotonEstable  = HIGH;
bool ultimoEstadoBoton   = HIGH;
unsigned long ultimoCambioBoton  = 0;
const unsigned long DEBOUNCE_MS  = 50;

// --- BUFFER DE RECEPCIÓN UART ---
byte bufferRX[8];
byte indexRX = 0;

void setup() {
  Serial.begin(9600);

  pinMode(pinBoton,    INPUT_PULLUP);
  pinMode(pinLedRojo,  OUTPUT);
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedAzul,  OUTPUT);
  pinMode(motorPWM,    OUTPUT);
  pinMode(motorIN1,    OUTPUT);
  pinMode(motorIN2,    OUTPUT);
  pinMode(motorSTBY,   OUTPUT);  // ← NUEVO: habilitar pin STBY

  // Dirección de giro del motor: adelante
  digitalWrite(motorIN1, HIGH);
  digitalWrite(motorIN2, LOW);

  // Sacar el TB6612FNG del modo standby (imprescindible)
  digitalWrite(motorSTBY, HIGH);  // ← NUEVO: sin esto el motor no gira

  // Estado inicial: motor apagado, LED verde (sistema listo)
  analogWrite(motorPWM, 0);
  setLED(false, true, false); // Verde
}

void loop() {
  manejarBoton();

  // Petición periódica de temperatura al Esclavo
  if (millis() - ultimaPeticion >= INTERVALO_PETICION) {
    enviarSolicitudTemp();
  }

  // Timeout: si el Esclavo no responde en TIMEOUT_RESPUESTA ms
  if (esperandoRespuesta && (millis() - tiempoEspera > TIMEOUT_RESPUESTA)) {
    errorComunicacion  = true;
    esperandoRespuesta = false;
  }

  controlarActuadores();
}

void serialEvent() {
  while (Serial.available()) {
    byte b = Serial.read();

    // Sincronización: solo aceptar SOF (0xAA) al inicio
    if (indexRX == 0 && b != 0xAA) continue;

    bufferRX[indexRX++] = b;

    if (indexRX < 4) continue; // Esperar mínimo SOF+ID+LEN+CHK

    byte id               = bufferRX[1];
    byte len              = bufferRX[2];
    byte tramaTotalEsperada = 4 + len;

    if (indexRX < tramaTotalEsperada) continue; // Trama incompleta

    // Validar checksum XOR
    byte chkRecibido  = bufferRX[tramaTotalEsperada - 1];
    byte chkCalculado = id ^ len;
    for (byte i = 0; i < len; i++) {
      chkCalculado ^= bufferRX[3 + i];
    }

    if (chkCalculado == chkRecibido) {
      latenciaRespuesta = micros() - tiempoEnvio; // Medir latencia
      procesarMensaje(id, len, &bufferRX[3]);
    }
    // Checksum incorrecto: trama descartada silenciosamente

    indexRX = 0; // Resetear buffer para la siguiente trama
  }
}

void procesarMensaje(byte id, byte len, byte* data) {
  switch (id) {

    case 0x02: // Respuesta de temperatura del Esclavo
      if (len == 2) {
        int tempX10       = ((int)data[0] << 8) | data[1];
        temperatura       = tempX10 / 10.0;
        errorComunicacion  = false;
        esperandoRespuesta = false;
      }
      break;

    case 0x04: // ACK: Esclavo confirmó recepción del comando de modo
      // Confirmación silenciosa, no requiere acción adicional
      break;

    default:
      break;
  }
}

void enviarSolicitudTemp() {
  byte chk     = 0x01 ^ 0x00;
  byte trama[] = {0xAA, 0x01, 0x00, chk};
  tiempoEnvio        = micros();
  Serial.write(trama, 4);
  ultimaPeticion     = millis();
  esperandoRespuesta = true;
  tiempoEspera       = millis();
}

void enviarComandoModo(bool activo) {
  byte valorModo = activo ? 0x01 : 0x00;
  byte chk       = 0x03 ^ 0x01 ^ valorModo;
  byte trama[]   = {0xAA, 0x03, 0x01, valorModo, chk};
  Serial.write(trama, 5);
}

void manejarBoton() {
  bool lecturaActual = digitalRead(pinBoton);

  if (lecturaActual != ultimoEstadoBoton) {
    ultimoCambioBoton = millis();
  }

  if ((millis() - ultimoCambioBoton) > DEBOUNCE_MS) {
    if (lecturaActual != estadoBotonEstable) {
      estadoBotonEstable = lecturaActual;

      if (estadoBotonEstable == LOW) { // Flanco de bajada = pulsación
        modoManual = !modoManual;
        enviarComandoModo(modoManual);
      }
    }
  }

  ultimoEstadoBoton = lecturaActual;
}

void controlarActuadores() {
  int velocidad = VELOCIDAD_NORMAL;

  if (errorComunicacion) {
    bool parpadeo = ((millis() / 250) % 2 == 0);
    setLED(parpadeo, false, false); // Rojo parpadeante
    velocidad = VELOCIDAD_NORMAL;

  } else if (modoManual) {
    setLED(false, false, true);     // Azul fijo — Modo Full
    velocidad = VELOCIDAD_FULL;

  } else {
    if (temperatura >= UMBRAL_TEMP) {
      setLED(true, false, false);   // Rojo fijo — alerta temperatura
      velocidad = VELOCIDAD_FULL;
    } else {
      setLED(false, true, false);   // Verde fijo — todo normal
      velocidad = VELOCIDAD_NORMAL;
    }
  }

  analogWrite(motorPWM, velocidad);
}

void setLED(bool r, bool g, bool b) {
  digitalWrite(pinLedRojo,  r ? HIGH : LOW);
  digitalWrite(pinLedVerde, g ? HIGH : LOW);
  digitalWrite(pinLedAzul,  b ? HIGH : LOW);
}
