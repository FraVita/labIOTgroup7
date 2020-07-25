const int TEMP_PIN = A1;    // pin analogico
const int B = 4275;         // valore per calcolo della temperatura presente su datasheet
const long int R0 = 100000; // valore per calcolo della temperatura presente su datasheet

//funzione di conversione del pin
float convert(float value) {
  float R = 1023.0/value-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // formula presente in datasheet
  return temperature; // temperatura convertita in Celsius
}

//inizializzazione
void setup() {
  pinMode(TEMP_PIN, INPUT);
  Serial.begin(9600);
  while(!Serial);
  Serial.println("lab 1.5 starting...");
}

//Lettura - Conversione - Stampa
void loop() {
  float temp = analogRead(TEMP_PIN);
  float celsius = convert(temp);
  Serial.print("Temperature: "); Serial.print(celsius); Serial.println("°C");
  delay(10*1e3);
}
