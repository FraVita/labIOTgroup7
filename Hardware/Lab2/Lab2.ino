//#include <TimerOne.h>
#include <LiquidCrystal_PCF8574.h>

//ASSEGNAZIONE DEI PIN AI SENSORI
const int MOTOR_PIN = 6;
const int LED_PIN = 5;
const int PIR_PIN = 8;
const int SOUND_PIN = 7;
const int TEMP_PIN = A1;
const int INT_LED_PIN = 13;

//VARIABILI GLOBALI
int f, h; //recuperano valori ottenuti dalle funzioni heat e fresh
LiquidCrystal_PCF8574 lcd(0x27);
bool someone = false; //variabile per dire se c'è qualcuno
int ts5=0; //timer per gestione schermo lcd
int ts6=0; //timer per gestione schermo lcd
bool personal = false; //sono stati scelti valori personalizzati?

//costanti set-point temperatura se è presente qualcuno
const int min_temp_th_def_somebody = 15;
const int max_temp_th_def_somebody = 20;
const int min_temp_ac_def_somebody = 25;
const int max_temp_ac_def_somebody = 30;

//costanti set-point temperatura se non è presente nessuno
const int min_temp_th_def_nobody = 10;
const int max_temp_th_def_nobody = 15;
const int min_temp_ac_def_nobody = 30;
const int max_temp_ac_def_nobody = 35;

//inizialmente setpoint assegnati a variabili con set-point non c'è nessuno
int min_temp_th = min_temp_th_def_nobody;
int max_temp_th = max_temp_th_def_nobody;
int min_temp_ac = min_temp_ac_def_nobody;
int max_temp_ac = max_temp_ac_def_nobody;

const long int B = 4275;    // datasheet sensore di temperatura
const long int R0 = 100000; // datasheet sensore di temperatura
int ts1 = millis(); // ultimo evento rilevato
int ts2 = millis(); // eventi multipli
int event_counter = 0; //conta eventi di suoni rilevati
long int timeout_pir = 1800*1e3; //30 minuti in ms (1800)
long int change_lcd = 5*1e3; //tempo per alternanza stampa lcd
long int n_sound_events = 50; //eventi di suono rilevati affinchè ci sia qualcuno
long int sound_interval = 600*1e3; //10 minuti (600)
long int timeout_sound = 3600*1e3; //60 minuti (3600)

void serialFlush() {
  while(Serial.available()) {
    Serial.read();
    delay(1);
  }
}

//FUNZIONE PER GESTIONE DI CONDIZIONAMENTO DELL'ARIA
//----------------------------------------------------------
int fresh(float temp) {
  float vel = map(temp, min_temp_ac, max_temp_ac, 0, 255);//rimappa il valore temp tra min_temp_ac e max... tra 0 e 255
  vel = constrain(vel, 0, 255);
//  Serial.print("Vel Fresh:"); Serial.print(vel); //debug
  analogWrite(MOTOR_PIN, int(vel));
  return vel;
}

//FUNZIONE PER GESTIONE DI CONDIZIONAMENTO DEL RISCALDAMENTO
//----------------------------------------------------------
int heat(float temp) {
  float th = map(temp, min_temp_th, max_temp_th, 255, 0);
  th = constrain(th, 0, 255);
//  Serial.print("th Fresh:"); Serial.print(th); //debug

  analogWrite(LED_PIN, int(th));
  return th;
}

float convert(float value) {
  float R = 1023.0/value-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; //datasheet
  return temperature;
}

//FUNZIONE PER RILEVAMENTO DEL SUONO
//----------------------------------------------------------
void sound_detected() {
  if(event_counter == 0)
    ts1 = millis(); //50 eventi in 10 minuti
  event_counter++;
  ts2 = millis();
  // Serial.print("Event "); Serial.println(event_counter);
  if( (ts2 - ts1) <= sound_interval && event_counter >= 50)  {
    Serial.println("Sound Dedected");
    isSomeone(); //se ci sono c'è qualcuno
    event_counter = 0;

  }
  else if ((ts2 - ts1) > sound_interval)
    event_counter = 0; //se il timer supera i 10 minuti il counter torna a 0
  
}

//FUNZIONE PER RILEVAMENTO PRESENZA TRAMITE PIR
//----------------------------------------------------------
void movement_detected() {
  Serial.println("Movement Dedected");
  //Timer1.initialize(timeout_pir); //timer per il pir, chiama l'interrupt con isntSomeone
  //Timer1.attachInterrupt(isntSomeone);
  ts1 = millis();
  isSomeone();
}

//FUNZIONE AVVIATA SE PRESENTE QUALCUNO
//----------------------------------------------------------
void isSomeone() {
  //se c'è qualcuno e non sono stati scelti valori personalizzati, usiamo questi setpoint di temperatura
  someone = true;
  Serial.println("C'è qualcuno");

  //SE SI SONO SCELTI VALORI PERSONALIZZATI SI UTILIZZANO QUELLI
  //----------------------------------------------------------
  if(!personal) {
    min_temp_th = min_temp_th_def_somebody;
    max_temp_th = max_temp_th_def_somebody;
    min_temp_ac = min_temp_ac_def_somebody;
    max_temp_ac = max_temp_ac_def_somebody;
  }

}


//FUNZIONE AVVIATA SE NON E' PRESENTE NESSUNO
//----------------------------------------------------------
void isntSomeone() {
  //Se non c'è nessuno e non sono presenti valori personalizzati, usiamo questi setpoint di temperatura
  someone = false;
  Serial.println("Non c'è nessuno");

  if(!personal) {
    min_temp_th = min_temp_th_def_nobody;
    max_temp_th = max_temp_th_def_nobody;
    min_temp_ac = min_temp_ac_def_nobody;
    max_temp_ac = max_temp_ac_def_nobody;
  }

}


//FUNZIONE PER STAMPA SU LCD (INFO DI TEMPERATURA E AZIONAMENTO ATTUATORI)
//----------------------------------------------------------
void stampaLCD(float temp, int v, int ht)
{
  v=map(v,0,255,0,100);
  ht=map(ht,0,255,0,100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temp); lcd.print((char)0xDF); lcd.print("C"); lcd.print(" Pres:"); lcd.print(someone);
  lcd.setCursor(0, 1);
  lcd.print("AC:"); lcd.print(v); lcd.print("% "); lcd.print("HT:"); lcd.print(ht); lcd.print("% ");
  delay(10*1e3);
}

//FUNZIONE PER STAMPA SU LCD (INFO SUI SET POINT DI TEMPERATURA)
//----------------------------------------------------------
void stampaLCDmM()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AC m:"); lcd.print(min_temp_ac); lcd.print(" M:"); lcd.print(max_temp_ac);
  lcd.setCursor(0, 1);
  lcd.print("HT m:"); lcd.print(min_temp_th); lcd.print(" M:"); lcd.print(max_temp_th);
  delay(10*1e3);
}


//FUNZIONE DI SETUP (INIZIALIZZAZIONE)
//----------------------------------------------------------
void setup() {
//INIZIALIZZAZIONE SCHEDA(PIN INTERNO 13 ACCEESSO -> SISTEMA OPERATIVO)
  Serial.begin(9600);
  while(!Serial);
  digitalWrite(INT_LED_PIN,LOW);
  Serial.println("Lab 2 Starting...");
  f=0;
  h=0;

//INIZIALIZZAZIONE LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("T: ");
  digitalWrite(INT_LED_PIN,HIGH);

  //INIZIALIZZAZIONE PIN
  Serial.println("\nPuoi modificare i set-point. Digita 'a' per AC o 'h' per HT. Per tornare ai valori di default digita 'r'.");
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(TEMP_PIN, INPUT);

//GESTIONE INTERRUPT(USATO SOLO QUELLO DI SUONO RILEVATO)
  //attachInterrupt(digitalPinToInterrupt(PIR_PIN), movement_detected, CHANGE); //interrupt quando viene rilevata una persone
  attachInterrupt(digitalPinToInterrupt(SOUND_PIN), sound_detected, FALLING);
}

//FUNZIONE CICLICA IN CUI VENGONO GESTITE:
//RILEVAMENTO TEMPERATURA E GESTIONE HEAT E FRESH
//VERIFICA PRESENZA O ASSENZA PERSONA CICLICAMENTE
//GESTIONE SENSORE PIR SENZA INTERRUPT
//ALTERNANZA DELLE STAMPE SU SCHERMO LCD TRAMITE VARIABILI TEMPORALI
//PERSONALIZZAZIONE DEI SETPOINT DI TEMPERATURA TRAMITE SERIALE
//----------------------------------------------------------
void loop() {

  int ts3 = millis();
  if((ts3 - ts1) >= timeout_sound && (ts3- ts1) >= timeout_pir )
    isntSomeone();
    //timer di non rilevamento suono (60 minuti)

 //RILEVAMENTO TEMPERATURA E GESTIONE HEAT E FRESH
  float temp = convert(analogRead(TEMP_PIN));
  if(temp >= min_temp_ac) f=fresh(temp);
  else if(temp <= max_temp_th) h=heat(temp);
  else {
    digitalWrite(LED_PIN,LOW);
    digitalWrite(MOTOR_PIN,LOW);
  }

//ALTERNANZA DELLE STAMPE SU SCHERMO LCD TRAMITE VARIABILI TEMPORALI
  ts6=millis();
  if((ts6-ts5)<change_lcd)
    stampaLCD(temp, f, h);
   else{
    stampaLCDmM();
    ts5=millis();
   }


//GESTIONE SENSORE PIR SENZA INTERRUPT
  int pir=digitalRead(PIR_PIN);
  if(pir==1)
    movement_detected();

//PERSONALIZZAZIONE DEI SETPOINT DI TEMPERATURA TRAMITE SERIALE
  if(Serial.available()) {

    char command = Serial.read();

    if(command == 'a' || command == 'A') {
      Serial.println("Digita 'm' per il minimo o 'M' per il massimo.");
      while(!Serial.available());
      char choise = Serial.read();
      if(choise == 'm') {
        Serial.println("Digita il valore.");
        while(!Serial.available());
        int value = Serial.parseInt();
        min_temp_ac = value;
        Serial.print("Modificato AC min --> "); Serial.println(min_temp_ac);
        personal = true;
      }
      else if(choise == 'M') {
        Serial.println("Digita il valore.");
        while(!Serial.available());
        int value = Serial.parseInt();
        max_temp_ac = value;
        Serial.print("Modificato AC max --> "); Serial.println(max_temp_ac);
        personal = true;
      }
      else Serial.println("Comando non valido");
    }

    else if(command == 'h' || command == 'H') {
      Serial.println("Digita 'm' per il minimo o 'M' per il massimo.");
      while(!Serial.available());
      char choise = Serial.read();
      if(choise == 'm') {
        Serial.println("Digita il valore.");
        while(!Serial.available());
        int value = Serial.parseInt();
        min_temp_th = value;
        Serial.print("Modificato HT min --> "); Serial.println(min_temp_th);
        personal = true;
      }
      else if(choise == 'M') {
        Serial.println("Digita il valore.");
        while(!Serial.available());
        int value = Serial.parseInt();
        max_temp_th = value;
        Serial.print("Modificato HT max --> "); Serial.println(max_temp_th);
        personal = true;
      }
      else Serial.println("Comando non valido");
    }

    else if(command == 'r' || command == 'R') {
      personal = false;
    }

    else Serial.println("Comando non valido");

  }

}
