#include <MQTTclient.h>
#include <ArduinoJson.h>
#include <Bridge.h>
#include <Process.h>

String url = "192.168.1.80:8080/registration";
int StartMillis = 0; //variabile utile per timestamp
const long int R0 = 100000; //variabile per conversione presente nel datasheet sensore di temperatura
const int B = 4275; //variabile per conversione presente nel datasheet sensore di temperatura
const int INT_LED_PIN = 13;
const int TEMP_PIN = A1;
const int capacity = 4*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(3) + 70;
DynamicJsonDocument doc_snd(capacity);
int countReg = 0;
int countTemp = 0;

//funzione di conversione della temperatura
float convert(float value) {
  float R = 1023.0/value-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; //datasheet
  return temperature;
}


String senMlEncodeRegistration()
{
  doc_snd.clear();
  doc_snd["device"]["deviceID"] = "Yun-IoT_Tech-Group7";
  doc_snd["device"]["endpoints"][0]= "/temperature/value";
  doc_snd["device"]["resources"][0] = "temperature";
  doc_snd["services"][0]["serviceID"] = "tempServiceMQTT";

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
  pinMode(TEMP_PIN, INPUT);
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

  if(countReg >= 100) {
    postReg();
    countReg = 0;
  }
  else {countReg++;}

  if(countTemp >= 10) {
    float temp = convert(analogRead(TEMP_PIN)); //lettura e conversione dato temperatura
    String message = senMlEncode("temperature", temp, "Cel"); //serializzo in formato SenMl la temperatura
    mqtt.publish(String("/temperature/value"), message); //invio come publisher al topic i dati serializzati

    //debug dati su porta seriale
    //Serial.print("Temp value: "); Serial.println(temp);
    Serial.print("Topic: "); Serial.println(String("/temperature/value"));
    Serial.print("Message: "); Serial.println(message);
    countTemp = 0;
  }
  else {countTemp++;}


  delay(1000);
}
