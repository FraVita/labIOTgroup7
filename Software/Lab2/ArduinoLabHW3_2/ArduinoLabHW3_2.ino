#include <ArduinoJson.h>
#include <Bridge.h>
#include <Process.h>

String url = "192.168.1.80:8080/registration";
const int INT_LED_PIN = 13;
const int capacity = 4*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(3) + 70;
DynamicJsonDocument doc_snd(capacity);

String senMlEncodeRegistration()
{
  doc_snd.clear();
  doc_snd["device"]["deviceID"] = "Yun-IoT_Tech-Group7";
  doc_snd["device"]["endpoints"][0]= "/temperature/value";
  doc_snd["device"]["resources"][0] = "temperature";
  doc_snd["services"][0] = "";


  String output;
  serializeJson(doc_snd, output);
  return output;

}

//inizializzazione
void setup() {
  Serial.begin(9600);
  pinMode(INT_LED_PIN, OUTPUT);
  digitalWrite(INT_LED_PIN, LOW);//Arduino inizializzato
  Bridge.begin();
  digitalWrite(INT_LED_PIN,HIGH);
  //A questo punto l'arduino è operativo

  while(!Serial);
  Serial.println("Starting...");
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
  
  postReg();
  
  delay(10000);
}
