#include <LiquidCrystal_PCF8574.h>

const int TEMP_PIN = A1;    // pin analogico
const int B = 4275;         // valore per calcolo della temperatura presente su datasheet
const long int R0 = 100000; // valore per calcolo della temperatura presente su datasheet

LiquidCrystal_PCF8574 lcd(0x27); // LCD address for 16 chars and 2 lines

// funzione di conversione in gradi Celsius di temperatura
float convert(float value) {
  float R = 1023.0/value-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // datasheet
  return temperature;
}

// inizializzazione
void setup() {
  pinMode(TEMP_PIN, INPUT);

  Serial.begin(9600);
  while(!Serial);
  Serial.println("lab 1.5 starting...");

  // inizializzazione schermo
  lcd.begin(16, 2); // caratteri visibili sullo schermo
  lcd.setBacklight(255); // accensione retroilluminazione
  lcd.home();
  lcd.clear();
  lcd.print("T: "); // scrittura in posizione (0,0) -> lcd.setCursor(0,0);
}

void loop() {
  //stampa tramite schermo lcd
  lcd.setCursor(3, 0); //sposto cursore direttamente su posizione riga 0 e colonna 3
  float temp = analogRead(TEMP_PIN); //lettura temperatura
  float celsius = convert(temp); //conversione temperatura
  lcd.print(celsius); lcd.print(" "); lcd.print((char)0xDF); lcd.print("C"); //0xDF = °
  delay(10*1e3);

}
