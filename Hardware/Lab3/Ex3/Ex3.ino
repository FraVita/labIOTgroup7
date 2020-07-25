#include <MQTTclient.h>
#include <ArduinoJson.h>
#include <TimerOne.h>
#include <Bridge.h>

int StartMillis = 0; //variabile utile per timestamp
const long int R0 = 100000; //variabile per conversione presente nel datasheet sensore di temperatura
const int B = 4275; //variabile per conversione presente nel datasheet sensore di temperatura
const int LED_PIN = 5;
const int INT_LED_PIN = 13;
int cnt=0;
const int TEMP_PIN = A1;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);
DynamicJsonDocument doc_rec(capacity);
String my_base_topic = String("/tiot/7"); //topic base prima di led e temperatrue

//funzione di conversione della temperatura
float convert(float value) {
  float R = 1023.0/value-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; //datasheet
  return temperature;
}

//funzione per leggere valore ricevuto da publisher e pilotare il led
void setLedValue(const String& topic, const String& subtopic, const String& message){
  DeserializationError err = deserializeJson(doc_rec, message);
  if(err)
  {
    Serial.print(F("deserializeJson() failed whit code"));
    Serial.println(err.c_str());
  }
  if (doc_rec["e"][0]["n"] == "led"){
    int value = doc_rec["e"][0]["v"];
    Serial.print("Led value: "); Serial.println(value); 
    
    if(value == 1 || value == 0){
      digitalWrite(LED_PIN, value);
    }
  }
}

//funzione di creazione formato SenMl per invio temperatura
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

//inzializzazione
void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  pinMode(INT_LED_PIN, OUTPUT);
  digitalWrite(INT_LED_PIN, LOW);//Arduino inizializzato
  Bridge.begin();
  digitalWrite(INT_LED_PIN,HIGH);
  //A questo punto l'arduino è operativo
  
  mqtt.begin("test.mosquitto.org",1883); //mi connetto al broker
  mqtt.subscribe(my_base_topic + String("/led"),setLedValue); //mi sottoscrivo al topic led
  
  while(!Serial);
  Serial.println("Starting...");
  StartMillis = millis(); //inzializzazione varaibile temporale per timestamp
  
}

void loop() {
  
  mqtt.monitor(); //funzione di monitoraggio per sezione led
  if(cnt==20)
  {
    float temp = convert(analogRead(TEMP_PIN)); //lettura e conversione dato temperatura
    String message = senMlEncode("temperature", temp, "Cel"); //serializzo in formato SenMl la temperatura
    mqtt.publish(my_base_topic + String("/temperature"), message); //invio come publisher al topic i dati serializzati
    //debug dati su porta seriale
    Serial.print("Temp value: "); Serial.println(temp); 
    Serial.print("Topic: "); Serial.println(my_base_topic + String("/temperature")); 
    Serial.print("Message: "); Serial.println(message); 
    cnt=0;
  }
  else
  {
    cnt=cnt+1;
  }
 
  
  
  delay(500);
}
