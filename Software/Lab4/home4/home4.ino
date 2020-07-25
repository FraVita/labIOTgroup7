#include <MQTTclient.h>
#include <Bridge.h>
#include <ArduinoJson.h>

const int LED_PIN1 = 5;
const int LED_PIN2 = 8;
const int LED_PIN3 = 12;
const int TEMP_PIN = A1;
const int INT_LED_PIN = 13;
const int HEAT = 6;
int contTemp=0;
const long int R0 = 100000; //variabile per conversione presente nel datasheet sensore di temperatura
const int B = 4275; //variabile per conversione presente nel datasheet sensore di temperatura
String my_base_topic="iot/domotic/control/service/svvgfs";
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_rec(capacity);
DynamicJsonDocument doc_snd(capacity);
int StartMillis=0;


//funzione di conversione della temperatura
float convert(float value) {
  float R = 1023.0/value-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; //datasheet
  return temperature;
}

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

//funzione per leggere valore ricevuto da publisher e accendere luci
void setLedValue(const String& topic, const String& subtopic, const String& message){
  Serial.println(message);
  DeserializationError err = deserializeJson(doc_rec, message);
  int value;
  int times;
  if(err)
  {
    Serial.print(F("deserializeJson() failed whit code"));
    Serial.println(err.c_str());
  }
  else
  {
    value = doc_rec["e"][0]["v"];
    times = doc_rec["e"][0]["t"];

    if (doc_rec["e"][0]["n"] == "Led1"){

       Serial.print("Led1 value: ");

        if(value == 1 || value == 0){
          digitalWrite(LED_PIN1, value);
        }
    }
    else if(doc_rec["e"][0]["n"] == "Led2"){

      Serial.print("Led2 value: ");

      if(value == 1 || value == 0){
        digitalWrite(LED_PIN2, value);
      }
    }
    else if(doc_rec["e"][0]["n"] == "Led3"){

      Serial.print("Led3 value: ");

     if(value == 1 || value == 0){
        digitalWrite(LED_PIN3, value);
      }

    }
    else if(doc_rec["e"][0]["n"] == "All"){

      Serial.print("All lights value: ");

     if(value == 1 || value == 0){
        digitalWrite(LED_PIN3, value);
        digitalWrite(LED_PIN2, value);
        digitalWrite(LED_PIN1, value);
      }

    }


   Serial.print(value); Serial.print(" Timestamp: "); Serial.println(times);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  pinMode(INT_LED_PIN, OUTPUT);
  digitalWrite(INT_LED_PIN, LOW);//Arduino inizializzato
  Bridge.begin();
  digitalWrite(INT_LED_PIN,HIGH);
  //A questo punto l'arduino è operativo
  mqtt.begin("mqtt.eclipse.org",1883); //mi connetto al broker
  mqtt.subscribe(my_base_topic + String("/light"),setLedValue); //mi sottoscrivo al topic led

  StartMillis=millis();
  while(!Serial);
  Serial.println("Starting...");

}

void loop() {
  // put your main code here, to run repeatedly:
  mqtt.monitor();

 if(contTemp==60)
  {
    float temp = convert(analogRead(TEMP_PIN)); //lettura e conversione dato temperatura
    String message = senMlEncode("temperature", temp, "\u00b0C"); //serializzo in formato SenMl la temperatura
    Serial.println(message);
    mqtt.publish(my_base_topic + String("/sensors/temperature"), message); //invio come publisher al topic i dati serializzati
    contTemp=0;
  }
  else
  {
    contTemp++;
  }
  delay(500);
}
