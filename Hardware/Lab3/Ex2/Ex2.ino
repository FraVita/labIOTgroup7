#include <ArduinoJson.h>
#include <TimerOne.h>
#include <Bridge.h>
#include <Process.h>

const long int R0 = 100000; //variabile per conversione presente nel datasheet sensore di temperatura
const int B = 4275; //variabile per conversione presente nel datasheet sensore di temperatura
const int INT_LED_PIN = 13;
int StartMillis = 0; //varaibile di inizializzazione per invio di timestamp corretto
const int TEMP_PIN = A1;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);
String url = "192.168.1.80:8080/log"; //url del server su cui inviare i dati di temperatura. Da modificare per ogni pc


//funzione di conversione temperatura
float convert(float value) {
  float R = 1023.0/value-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; //datasheet
  return temperature;
}

//funzione per creare file SenMl con temperature
String senMlEncode(String res, float v, String unit) {
  doc_snd.clear();
  doc_snd["bn"] = "Yun";
  doc_snd["e"][0]["n"] = res;
  doc_snd["e"][0]["t"] = (millis()-StartMillis) / 1000;
  doc_snd["e"][0]["v"] = v;
  if(unit != "") {
    doc_snd["e"][0]["u"] = unit;
  } else {
    doc_snd["e"][0]["u"] = (char*) NULL;
  }
  
  String output;
  serializeJson(doc_snd, output);
  return output;
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
  p.addParameter(url);
  p.run();
  return p.exitValue();
  
}

//inizializzazione
void setup() {
  
  Serial.begin(9600);
  pinMode(TEMP_PIN, INPUT);
  pinMode(INT_LED_PIN, OUTPUT);
  digitalWrite(INT_LED_PIN, LOW);//Arduino inizializzato
  Bridge.begin();
  digitalWrite(INT_LED_PIN,HIGH);
  //codice in questo momento operativo
  
  while(!Serial);
  Serial.println("Starting...");
  StartMillis = millis(); //inzializzazione varaibile temporale
  
}


void loop() {
  float temp = convert(analogRead(TEMP_PIN)); //lettura e conversione dato proveniente da sensore di temperatura
  String json = senMlEncode(F("temperature"), temp, F("Cel")); //creazione dati
  int request=postRequest(json); //invio dati tramite richiesta json
  if(request==0)
    Serial.println("0 ---> Richiesta recapitata al server");
  else{
    Serial.print(request); Serial.println(" --->Errore!!! Richiesta non recepita dal server");
  }
  delay(1000); //ogni secondo viene inviato un nuovo dato di log
  
}
