// --- PINES DE CONEXIÓN ---
const int pinTemp = A0;
const int pinBoton = 2;
const int pinLedVerde = 5;
const int pinLedRojo = 6;

// Controlador de motor TB6612FNG
const int motorPWM = 11;
const int motorIN1 = 9;
const int motorIN2 = 8;

// --- VARIABLES DEL SISTEMA ---
float umbral = 20.0;
bool modoManual = false;
int velocidadMotor = 0;

// Variables para el Debounce (Antirrebote) del botón
bool estadoBotonEstable = HIGH; 
bool ultimoEstadoBoton = HIGH;
unsigned long ultimoTiempoCambio = 0;
unsigned long tiempoDebounce = 50; // 50 milisegundos de filtro

void setup() {
  Serial.begin(9600); // Iniciar comunicación UART
  
  pinMode(pinBoton, INPUT_PULLUP);
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedRojo, OUTPUT);
  
  pinMode(motorPWM, OUTPUT);
  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);
  
  // Configurar dirección del motor (Hacia adelante por defecto)
  digitalWrite(motorIN1, HIGH);
  digitalWrite(motorIN2, LOW);
}

void loop() {
  manejarBoton(); // Revisar si el usuario está apretando el botón
  
  // 1. Leer y calcular temperatura
  int lecturaCruda = analogRead(pinTemp);
  float voltaje = lecturaCruda * (5.0 / 1023.0);
  float temperatura = (voltaje - 0.5) * 100.0;
  
  // 2. Lógica de Control
  if (modoManual) {
    // Si el botón está presionado: Forzar velocidad al máximo
    velocidadMotor = 180;
    digitalWrite(pinLedRojo, HIGH);
    digitalWrite(pinLedVerde, LOW);
  } else {
    // Si no está presionado: Control Automático
    if (temperatura < umbral) {
      velocidadMotor = 0; // Apagado
      digitalWrite(pinLedRojo, LOW);
      digitalWrite(pinLedVerde, HIGH); // Verde indica reposo
    } else if (temperatura < 25.0) {
      // Rampa de aceleración entre 15°C y 25°C
      velocidadMotor = map(temperatura, 15, 25, 90, 180);
      digitalWrite(pinLedRojo, HIGH); // Rojo indica movimiento
      digitalWrite(pinLedVerde, LOW);
    } else {
      velocidadMotor = 180; // Tope máximo de seguridad
      digitalWrite(pinLedRojo, HIGH);
      digitalWrite(pinLedVerde, LOW);
    }
  }
  
  // 3. Ejecutar el movimiento
  analogWrite(motorPWM, velocidadMotor);
  
  // 4. Enviar Telemetría (Trama UART)
  Serial.print("$");
  Serial.print(modoManual ? "M" : "A");
  Serial.print(",");
  Serial.print(temperatura);
  Serial.print(",");
  Serial.print(velocidadMotor);
  Serial.println("*");
  
  delay(100); // Pequeña pausa para estabilizar las lecturas
}

// Función para filtrar el ruido mecánico del botón (Modo Interruptor)
void manejarBoton() {
  bool lecturaActual = digitalRead(pinBoton);
  
  // Si hay un cambio (aunque sea ruido), resetear el cronómetro
  if (lecturaActual != ultimoEstadoBoton) {
    ultimoTiempoCambio = millis();
  }
  
  // Si el estado se ha mantenido estable por más del tiempo de debounce
  if ((millis() - ultimoTiempoCambio) > tiempoDebounce) {
    // Si el estado estable cambió respecto al registrado
    if (lecturaActual != estadoBotonEstable) {
      estadoBotonEstable = lecturaActual;
      
      // Lógica de Interruptor: Un toque invierte el modo
      if (estadoBotonEstable == LOW) {
        modoManual = !modoManual; // Cambia de automático a manual, o viceversa
      }
    }
  }
  // Guardar el estado actual para la siguiente vuelta
  ultimoEstadoBoton = lecturaActual;
}