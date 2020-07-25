const int MOTOR_PIN = 6;
const float stp = 255.0/10.0; // step incremento/decremento velocità
float vel = 0; // velocità effettiva del motore

void serialFlush() {
  while(Serial.available()) {
    Serial.read();
    delay(1);
  }
}

// funzione per incremento velocità
void increaseSpeed() {
  if(vel >= 255) {
    Serial.println("Already at max speed");
    return;
  }
  vel += stp;
  vel = constrain(vel, 0, 255);
  analogWrite(MOTOR_PIN, int(vel));
  Serial.print("Increasing speed: "); Serial.println(vel);
}

// funzione per decremento velocità
void decreaseSpeed() {
  if(vel <= 0) {
    Serial.println("Already at min speed");
    return;
  }
  vel -= stp;
  vel = constrain(vel, 0, 255);
  analogWrite(MOTOR_PIN, int(vel));
  Serial.print("Decreasing speed: "); Serial.println(vel);
}

// inizializzazione
void setup() {
  // inizializzazione pin motore
  pinMode(MOTOR_PIN, OUTPUT);
  analogWrite(MOTOR_PIN, 0);

  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 1.4 starting...");
  Serial.println("Inserire '+' per incrementare velocita' ventola o '-' per decrementarla");
}

void loop() {
  //richiesta di comando da utente su seriale
  while(!Serial.available());
  char command = Serial.read();
  if(command == '+') increaseSpeed();
  else if (command == '-') decreaseSpeed();
  else Serial.println("Invalid command.");
  serialFlush();
}
