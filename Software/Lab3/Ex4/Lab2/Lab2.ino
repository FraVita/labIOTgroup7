#include <LiquidCrystal_PCF8574.h>
#include <MQTTclient.h>
#include <Bridge.h>
#include <Process.h>

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
String url = "192.168.1.53:8080/registration";
bool someone = false; //variabile per dire se c'è qualcuno
int ts5=0; //timer per gestione schermo lcd
int ts6=0; //timer per gestione schermo lcd
int countReg = 0;//Per fare registrazione
int countPir = 0;//Per vedere pir ogni 3 secondi
bool personal = false; //sono stati scelti valori personalizzati?
int StartMillis=0;
int countTemp=0;

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
long int n_sound_events = 50; //eventi di suono rilevati affinchè ci sia qualcuno
long int sound_interval = 600*1e3; //10 minuti (600)
long int timeout_sound = 3600*1e3; //60 minuti (3600)

void serialFlush() {
  while(Serial.available()) {
    Serial.read();
    delay(1);
  }
}



//Invio dei dati tramite richiesta POST
int postRequest(String data){
  Process p;
  p.begin("curl");
  p.addParameter("-H");
  p.addParameter("Content-Type: application/json");
  p.addParameter("-X");
  p.addParameter("POST");
  p.addParameter("-d");
  p.addParameter(data);
  Serial.println(data);
  p.addParameter(url);
  p.run();

  
  return p.exitValue();

  

}

void postReg()
{
  
  String message=String(F("{\"device\": { \"deviceID\":\"Yun-IoT_Tech-Group7\",\"endpoints\":[\"/temp\",\"/move\",\"/sound\"]}}"));
  Serial.print(F("Message: ")); 
  Serial.println(postRequest(message));
  
}




//Stampo messaggio ricevuto su LCD
void EncodeStampaLCD(const String& Topic, const String& subtopic,const String& message)
{
    
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print(message);
    
  
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
  if( (ts2 - ts1) <= sound_interval && event_counter >= 50)  {
    //String message = senMlEncode("sound", 0, "sound"); //serializzo in formato SenMl il suono non rilevato
    String message = String("{'bn':'Yun','e':[{'v':")+String("1")+String(", 't':0, 'u':'Sound','n':'sound'")+String("}]}");

    mqtt.publish( String("sound"), message); //invio come publisher al topic i dati serializzati
    Serial.println(F("Sound Dedected"));
    Serial.print(F("Topic: ")); Serial.println(F("sound"));
    Serial.print(F("Message: ")); Serial.println(message);
    isSomeone(); //se ci sono c'è qualcuno
    event_counter = 0;

  }
  else if ((ts2 - ts1) > sound_interval)
    event_counter = 0; //se il timer supera i 10 minuti il counter torna a 0
  

}

//FUNZIONE PER RILEVAMENTO PRESENZA TRAMITE PIR
//----------------------------------------------------------
void movement_detected() {
  Serial.println(F("Movement Dedected"));
  //Timer1.initialize(timeout_pir); //timer per il pir, chiama l'interrupt con isntSomeone
  //Timer1.attachInterrupt(isntSomeone);
  ts1 = millis();
  //String message = senMlEncode("movement", 1, "Presence"); //serializzo in formato SenMl il PIR ha detectato
  String message = String("{\"bn\":\"Yun\",\"e\":[{\"v\":")+String("1")+String(", \"t\":0, \"u\":\"Presence\",\"n\":\"movement\"")+String("}]}");

  mqtt.publish( String(F("move")), message); //invio come publisher al topic i dati serializzati
  Serial.print(F("Topic: ")); Serial.println(F("/move"));
  Serial.print(F("Message: ")); Serial.println(message);
  isSomeone();
}

//FUNZIONE AVVIATA SE PRESENTE QUALCUNO
//----------------------------------------------------------
void isSomeone() {
  //se c'è qualcuno e non sono stati scelti valori personalizzati, usiamo questi setpoint di temperatura
  someone = true;
  Serial.println(F("C'è qualcuno"));

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
  Serial.println(F("Non c'è nessuno"));
    
    String message = String("{\"bn\":\"Yun\",\"e\":[{\"v\":")+String("0")+String(", \"t\":0, \"u\":\"No Presence\",\"n\":\"movement\"")+String("}]}");
    mqtt.publish(String("move"), message); //invio come publisher al topic i dati serializzati

    
    message = String("{\"bn\":\"Yun\",\"e\":[{\"v\":")+String("0")+String(", \"t\":0, \"u\":\"No sound\",\"n\":\"sound\"")+String("}]}");

    mqtt.publish( String("sound"), message); //invio come publisher al topic i dati serializzati
    

  if(!personal) {
    min_temp_th = min_temp_th_def_nobody;
    max_temp_th = max_temp_th_def_nobody;
    min_temp_ac = min_temp_ac_def_nobody;
    max_temp_ac = max_temp_ac_def_nobody;
  }

}




//FUNZIONE DI SETUP (INIZIALIZZAZIONE)
//----------------------------------------------------------
void setup() {
  
//INIZIALIZZAZIONE SCHEDA(PIN INTERNO 13 ACCEESSO -> SISTEMA OPERATIVO)
  Serial.begin(9600);
  while(!Serial);
  digitalWrite(INT_LED_PIN,LOW);
  
  f=0;
  h=0;
  
//INIZIALIZZAZIONE PIN
  //Serial.println(F("\nPuoi modificare i set-point. Digita 'a' per AC o 'h' per HT. Per tornare ai valori di default digita 'r'."));
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(TEMP_PIN, INPUT);

  Bridge.begin();
  while(!Serial);
  Serial.println(F("Lab 2 Starting..."));
  
//INIZIALIZZAZIONE MQTT
  
  mqtt.begin("mqtt.eclipse.org",1883);
  mqtt.subscribe("/messageLCD",EncodeStampaLCD);
  
  StartMillis=millis();

//INIZIALIZZAZIONE LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("MESSAGE: ");
  digitalWrite(INT_LED_PIN,HIGH);

//GESTIONE INTERRUPT(USATO SOLO QUELLO DI SUONO RILEVATO)
  //attachInterrupt(digitalPinToInterrupt(SOUND_PIN), sound_detected, RISING);
  //Serial.println(F("Setup finito"));
  
  
}

//FUNZIONE CICLICA IN CUI VENGONO GESTITE:
//RILEVAMENTO TEMPERATURA E GESTIONE HEAT E FRESH
//VERIFICA PRESENZA O ASSENZA PERSONA CICLICAMENTE
//GESTIONE SENSORE PIR SENZA INTERRUPT
//STAMPA SU SCHERMO LCD
//----------------------------------------------------------
void loop() {
  mqtt.monitor();
  
  //REGISTRAZIONE
  if(countReg >= 30) {
    postReg();
    countReg = 0;
  }
  else {countReg++;}
  
  int ts3 = millis();
  if((ts3 - ts1) >= timeout_sound && (ts3- ts1) >= timeout_pir )
    isntSomeone();
    //timer di non rilevamento suono (60 minuti)

 //RILEVAMENTO TEMPERATURA E GESTIONE HEAT E FRESH CON INVIO DATI SU TEMPERATURA(non ogni secondo ma ogni 10 secondi)
 float temp = convert(analogRead(TEMP_PIN));
 if(countTemp >= 10) {
   //Serial.println("Inizio Loop");
    //String message = senMlEncode("temperature", temp, "Cel"); //serializzo in formato SenMl la temperatura
    String message = String("{\"bn\":\"Yun\",\"e\":[{\"v\":")+String(temp)+String(",\"t\":0,\"u\":\"Cel\",\"n\":\"temperature\"")+String("}]}");
    mqtt.publish("temp", message); //invio come publisher al topic i dati serializzati
    
    //debug dati su porta seriale
    Serial.print(F("Topic: ")); Serial.println(F("/temp"));
    Serial.print(F("Message: ")); Serial.println(message);
    countTemp = 0;
  }
  else {countTemp++;}
  
  if(temp >= min_temp_ac) f=fresh(temp);
  else if(temp <= max_temp_th) h=heat(temp);
  else {
    digitalWrite(LED_PIN,LOW);
    digitalWrite(MOTOR_PIN,LOW);
  }

//GESTIONE SENSORE PIR SENZA INTERRUPT
  if(countPir>=6)
  {
    int pir=digitalRead(PIR_PIN);
    if(pir==1)
    {
      movement_detected();
      countPir=0;
    }
    
  }
  else{countPir++;}

  delay(500);

}
