#include <TimerOne.h>
#define RED_LED 11
#define GREEN_LED 12

const float redLedPeriod = 1.5;   // tempo del semiperiodo led rosso
const float greenLedPeriod = 3.5; // tempo del semiperiodo led verde
bool ledGreenStatus = LOW;
bool ledRedStatus = LOW;

// Funzione gestita da interrupt alla scadenza di Timer1
void greenLedOn() {
  ledGreenStatus = !ledGreenStatus;
  digitalWrite(GREEN_LED, ledGreenStatus);
}

// Inizializzazione
void setup() {
  // settaggio pin
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // inizializzazione timer con interrupt legato alla funzione di gestione pin verde
  Timer1.initialize(greenLedPeriod * 1e06);
  Timer1.attachInterrupt(greenLedOn);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

}

void loop() {
  // gestione nel loop dell'alternanza (led on/off) del led verde tramite delay
  ledRedStatus = !ledRedStatus;
  digitalWrite(RED_LED, ledRedStatus);
  delay(redLedPeriod * 1e03);

}
