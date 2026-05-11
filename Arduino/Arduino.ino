const int pinSensor = A0;
const int pinBoton = 2;

const int ledVerde = 7;
const int ledRojo = 6;

const int motorPWM = 9;
const int motorIN1 = 8;
const int motorIN2 = 4;

float umbral = 30.0;

bool modoManual = false;
bool ultimoEstadoBoton = HIGH;
bool estadoBotonEstable = HIGH;

unsigned long ultimoTiempoCambio = 0;
const unsigned long tiempoDebounce = 50;

void setup() {
  Serial.begin(9600);

  pinMode(pinBoton, INPUT_PULLUP);

  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);

  pinMode(motorPWM, OUTPUT);
  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);

  digitalWrite(motorIN1, HIGH);
  digitalWrite(motorIN2, LOW);
}