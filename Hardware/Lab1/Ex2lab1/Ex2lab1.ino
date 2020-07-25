#include <TimerOne.h>
#define RED_LED 11
#define GREEN_LED 12

const float redLedPeriod = 1.5;   // semiperiodo led rosso
const float greenLedPeriod = 3.5; // semiperiodo led verde
volatile bool ledStatusGreen = LOW;
volatile bool ledStatusRed = LOW;

void serialFlush() {
  while(Serial.available()){
    Serial.read();
    delay(1);
  }
}

// Funzione di stampa stato led su porta seriale
void printStatus() {
  char colorled = Serial.read();

    if(colorled == 'R' || colorled == 'r') {

      if(ledStatusRed)
        Serial.println("Red Led Status: on");
      else
        Serial.println("Red Led Status: off");
      serialFlush();
    }
    else if (colorled == 'G' || colorled == 'g') {
      if(ledStatusRed)
        Serial.println("Green Led Status: on");
      else
        Serial.println("Green Led Status: off");
      serialFlush();
    }

    else {
      Serial.println("Comando non valido");
    }

    Serial.println("Inserisci 'g' per sapere stato led verde o 'r' per sapere stato led rosso");

}


void greenLedOn() {
  ledStatusGreen = !ledStatusGreen;
  digitalWrite(GREEN_LED, ledStatusGreen);
}

// inizializzazione
void setup() {
  Serial.begin(9600); // baud rate
  while(!Serial);
  Serial.println("LAB1.2 Starting...");

  // settaggio pin
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // inizializzazione timer con interrupt legato alla funzione di gestione pin verde
  Timer1.initialize(greenLedPeriod * 1e06);
  Timer1.attachInterrupt(greenLedOn);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  Serial.println("Inserisci 'g' per sapere stato led verde o 'r' per sapere stato led rosso");
}

void loop() {

  delay( redLedPeriod * 1e03);
  if(Serial.available()) printStatus();
  ledStatusRed = !ledStatusRed;
  digitalWrite(RED_LED, ledStatusRed);
}
