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

// Variables Botón
bool estadoBotonEstable = HIGH; 
bool ultimoEstadoBoton = HIGH;
unsigned long ultimoTiempoCambio = 0;
unsigned long tiempoDebounce = 50; 

// --- VARIABLES UART Y PROTOCOLO ---
unsigned long ultimaPeticion = 0;
const unsigned long intervaloPeticion = 1500; // Pedir datos cada 1.5 segundos (más calmado)
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
  
  // PARCHE TINKERCAD: Forzamos la ejecución de la lectura serial
  serialEvent(); 
  
  if (millis() - ultimaPeticion >= intervaloPeticion) {
    pedirTemperatura();
    ultimaPeticion = millis();
    esperandoRespuesta = true;
    tiempoEspera = millis();
  }
  
  // TIMEOUT AUMENTADO: Le damos 1000ms al simulador para responder
  if (esperandoRespuesta && (millis() - tiempoEspera > 1000)) {
    errorComunicacion = true; 
    esperandoRespuesta = false;
  }
  
  if (errorComunicacion) {
    velocidadMotor = 0;
    digitalWrite(pinLedVerde, LOW);
    
    // Parpadeo Rojo de Error
    if ((millis() / 250) % 2 == 0) {
      digitalWrite(pinLedRojo, HIGH);
    } else {
      digitalWrite(pinLedRojo, LOW);
    }
  } else {
    // Funcionamiento normal sincronizado
    if (modoManual) {
      velocidadMotor = 180;
      digitalWrite(pinLedRojo, HIGH);
      digitalWrite(pinLedVerde, LOW);
    } else {
      if (temperatura < umbral) {
        velocidadMotor = 0;
        digitalWrite(pinLedRojo, LOW);
        digitalWrite(pinLedVerde, HIGH); // Debería encender verde aquí!
      } else if (temperatura < 40.0) {
        velocidadMotor = map(temperatura, 20, 40, 90, 180);
        digitalWrite(pinLedRojo, HIGH);
        digitalWrite(pinLedVerde, LOW);
      } else {
        velocidadMotor = 180;
        digitalWrite(pinLedRojo, HIGH);
        digitalWrite(pinLedVerde, LOW);
      }
    }
  }
  
  analogWrite(motorPWM, velocidadMotor);
}

void pedirTemperatura() {
  byte sof = 0xAA;
  byte id = 0x01;
  byte len = 0x00;
  byte chk = id ^ len;
  
  Serial.write(sof);
  Serial.write(id);
  Serial.write(len);
  Serial.write(chk);
}

void serialEvent() {
  // Mientras tengamos datos, procesamos
  while (Serial.available() > 0) {
    if (Serial.peek() == 0xAA) {
      // Si encontramos el inicio, salimos para procesar el resto en el loop
      return; 
    } else {
      // Si no es 0xAA, es basura, la eliminamos y seguimos buscando
      Serial.read(); 
    }
  }
}
void manejarBoton() {
  bool lecturaActual = digitalRead(pinBoton);
  if (lecturaActual != ultimoEstadoBoton) {
    ultimoTiempoCambio = millis();
  }
  if ((millis() - ultimoTiempoCambio) > tiempoDebounce) {
    if (lecturaActual != estadoBotonEstable) {
      estadoBotonEstable = lecturaActual;
      if (estadoBotonEstable == LOW) {
        modoManual = !modoManual;
      }
    }
  }
  ultimoEstadoBoton = lecturaActual;
}