#include <ArduinoJson.h>
#include <TimerOne.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

//dichiarazione variabili
int StartMillis = 0; //varaibile di inizializzazione per invio di timestamp corretto
const long int R0 = 100000; //varaibili presenti in datasheet sensore di temperatura
const int B = 4275;  //varaibili presenti in datasheet sensore di temperatura
const int LED_PIN = 5;
const int INT_LED_PIN = 13;
const int TEMP_PIN = A1;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;

DynamicJsonDocument doc_snd(capacity);
BridgeServer server;

//funzione di conversione della temperatura
float convert(float value) {
  float R = 1023.0/value-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; //datasheet
  return temperature;
}

//funzione di creazione di documento Sen ML (molto simile a JSON)
String senMlEncode(String res, float v, String unit) {
  doc_snd.clear();
  doc_snd["bn"] = "Yun";
  doc_snd["e"][0]["n"] = res;
  doc_snd["e"][0]["t"] = (millis()-StartMillis) / 1000; //timestamp del valore di temperatura
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

//funzione di stampa della risposta del client
void printResponse(BridgeClient client, int code, String body){
  client.println("Status: " + String(code));
  if(code == 200) {
    client.println(F("Content-type: application/json; charset=utf-8"));
    client.println();
    client.println(body);
  }
}


void process(BridgeClient client){
  
  String command = client.readStringUntil('/');
  command.trim();

  if(command == "led") {
    int val = client.parseInt();
    if(val == 0 || val ==1) {
      digitalWrite(LED_PIN, val);
      printResponse(client, 200, senMlEncode(F("led"), val, F("")));
    }
    else {
      printResponse(client, 400, "Bad Request");
    }
  }
  else if(command == "temperature"){
    float temp = convert(analogRead(TEMP_PIN));
    printResponse(client, 200, senMlEncode(F("temperature"), temp, F("Cel")));
  }
  else {
      printResponse(client, 404, "Request Not Found");
    }    
}

//inziailizzazione
void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  pinMode(INT_LED_PIN, OUTPUT);
  digitalWrite(INT_LED_PIN, LOW);//Arduino inizializzato
  Bridge.begin();
  digitalWrite(INT_LED_PIN,HIGH);
  server.listenOnLocalhost();
  server.begin();
  
  while(!Serial);
  Serial.print("Starting...");
  StartMillis = millis();
  
}

void loop() {
 BridgeClient client = server.accept();
 if (client){
    process(client);
    client.stop(); //chiusura connessione da parte del client
  }
  delay(50);
}
