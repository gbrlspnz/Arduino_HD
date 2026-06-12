// --- PINES DE CONEXIÓN ---
const int pinBoton = 2;
const int pinLedVerde = 5;  
const int pinLedRojo = 6;    
const int motorPWM = 11;
const int motorIN1 = 9;
const int motorIN2 = 8;

// --- VARIABLES DEL SISTEMA ---
float temperatura = 0.0;
float umbral = 20.0;
bool modoManual = false;
int velocidadMotor = 0;

// Antirrebote del Botón
bool estadoBotonEstable = HIGH; 
bool ultimoEstadoBoton = HIGH;
unsigned long ultimoTiempoCambio = 0;
unsigned long tiempoDebounce = 50; 

// Control UART
unsigned long ultimaPeticion = 0;
bool esperandoRespuesta = false;
unsigned long tiempoEspera = 0;
bool errorComunicacion = false;

void setup() {
  Serial.begin(9600); 
  
  pinMode(pinBoton, INPUT_PULLUP);
  pinMode(pinLedRojo, OUTPUT);
  pinMode(pinLedVerde, OUTPUT);
  pinMode(motorPWM, OUTPUT);
  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);
  
  digitalWrite(motorIN1, HIGH);
  digitalWrite(motorIN2, LOW);
}

void loop() {
  manejarBoton();
  procesarUART(); 
  
  // 1. Petición de datos al Esclavo (cada 1.5s)
  if (millis() - ultimaPeticion >= 1500) {
    byte trama[] = {0xAA, 0x01, 0x00, 0x01};
    Serial.write(trama, 4);
    ultimaPeticion = millis();
    esperandoRespuesta = true;
    tiempoEspera = millis();
  }
  
  // 2. Timeout de seguridad
  if (esperandoRespuesta && (millis() - tiempoEspera > 2000)) {
    errorComunicacion = true; 
    esperandoRespuesta = false;
  }
  
  // 3. Lógica de Actuadores
  if (errorComunicacion) {
    velocidadMotor = 0;
    digitalWrite(pinLedVerde, LOW);
    // Parpadeo rojo de error
    digitalWrite(pinLedRojo, ((millis() / 250) % 2 == 0) ? HIGH : LOW);
  } else {
    if (modoManual) {
      // Modo Manual Activo
      velocidadMotor = 180;
      digitalWrite(pinLedRojo, HIGH);
      digitalWrite(pinLedVerde, LOW);
    } else {
      // Modo Automático
      if (temperatura < umbral) {
        velocidadMotor = 0;
        digitalWrite(pinLedRojo, LOW);
        digitalWrite(pinLedVerde, HIGH); // Todo normal (Verde)
      } else {
        velocidadMotor = 180; 
        digitalWrite(pinLedRojo, HIGH); // Alerta temperatura (Rojo)
        digitalWrite(pinLedVerde, LOW);
      }
    }
  }
  
  analogWrite(motorPWM, velocidadMotor);
}

// --- FUNCIONES SECUNDARIAS ---

void procesarUART() {
  while (Serial.available() >= 5) {
    if (Serial.peek() == 0xAA) {
      Serial.read(); 
      byte id = Serial.read();
      byte len = Serial.read();
      byte data = Serial.read();
      byte chkRecibido = Serial.read();
      
      if ((id ^ len ^ data) == chkRecibido && id == 0x02) {
        temperatura = (float)data; 
        errorComunicacion = false; 
        esperandoRespuesta = false;
      }
    } else {
      Serial.read(); 
    }
  }
}

void manejarBoton() {
  bool lecturaActual = digitalRead(pinBoton);
  if (lecturaActual != ultimoEstadoBoton) { ultimoTiempoCambio = millis(); }
  if ((millis() - ultimoTiempoCambio) > tiempoDebounce) {
    if (lecturaActual != estadoBotonEstable) {
      estadoBotonEstable = lecturaActual;
      if (estadoBotonEstable == LOW) { modoManual = !modoManual; }
    }
  }
  ultimoEstadoBoton = lecturaActual;
}