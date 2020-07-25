
// pin associati a led
const int PIR_PIN = 7;
const int LED_PIN = 5;
int counter = 0; // contatore di movimenti rilevati

// funzione scatenata da interrupt per rilevamento movimento pin
void count() {
  int stat = digitalRead(PIR_PIN);
  digitalWrite(LED_PIN, stat);
  if(stat) counter++;
}

// inizializzazione
void setup() {
  // inizializzazione dei pin
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // interrupt per rilevamento movimento pir
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), count, CHANGE);

  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 1.3 starting...");
}

void loop() {
  // stampa del contatore di moviemnti rilevati
  Serial.print("Total people count: "); Serial.println(counter);
  delay(30*1e3);
}
