#include <MQTTclient.h>
#include <ArduinoJson.h>
#include <Bridge.h>
#include <Process.h>

String url = "192.168.1.80:8080/registration";
int StartMillis = 0; //variabile utile per timestamp

const int LED_PIN = 5;
const int INT_LED_PIN = 13;
//const int TEMP_PIN = A1;
//const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
const int capacity = 4*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(3) + 70;
DynamicJsonDocument doc_snd(capacity);
//DynamicJsonDocument doc_sndReg(capacityReg);
//String my_base_topic = String("/tiot/7"); //topic base prima di led e temperatrue
int countReg = 0;


//funzione per leggere valore ricevuto da publisher e pilotare il led
void setLedValue(const String& topic, const String& subtopic, const String& message){
  DeserializationError err = deserializeJson(doc_snd, message);
  if(err)
  {
    Serial.print(F("deserializeJson() failed whit code"));
    Serial.println(err.c_str());
  }
  if (doc_snd["e"][0]["n"] == "led"){
    int value = doc_snd["e"][0]["v"];
    Serial.print("Led value: "); Serial.println(value);

    if(value == 1 || value == 0){
      digitalWrite(LED_PIN, value);
    }
  }
}


String senMlEncodeRegistration()
{
  doc_snd.clear();
  doc_snd["device"]["deviceID"] = "Yun-IoT_Tech-Group7";
  doc_snd["device"]["endpoints"][0]= "/led";
  doc_snd["device"]["resources"][0] = "temperature";
  doc_snd["services"][0]["serviceID"] = "ledMQTT";
  //doc_snd["services"][0]["description"] = "Temperature_sent_with_MQTT_request";
  //doc_snd["services"][0]["endpoints"][0] = "/temperature/value";


  String output;
  serializeJson(doc_snd, output);
  return output;

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
  //pinMode(LED_PIN, OUTPUT);
  //pinMode(TEMP_PIN, INPUT);
  pinMode(INT_LED_PIN, OUTPUT);
  digitalWrite(INT_LED_PIN, LOW);//Arduino inizializzato
  Bridge.begin();
  digitalWrite(INT_LED_PIN,HIGH);
  //A questo punto l'arduino è operativo

  while(!Serial);
  Serial.println("Starting...");
  
  postReg();

  delay(10);

  mqtt.begin("mqtt.eclipse.org",1883); //mi connetto al broker
  Serial.println("Connesso al broker MQTT");
  mqtt.subscribe(/*my_base_topic + */String("/led"),setLedValue); //mi sottoscrivo al topic led
  
  StartMillis = millis(); //inzializzazione varaibile temporale per timestamp

}

void postReg()
{
  Serial.println(senMlEncodeRegistration());
  postRequest(senMlEncodeRegistration());
  Serial.println("Invio registration"); //invio dati tramite richiesta
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

void loop() {

  mqtt.monitor(); //funzione di monitoraggio per sezione led
  
  if(countReg >= 100) {
    postReg();
    countReg = 0;
  }
  else {countReg++;}

  
  delay(1000);
}
