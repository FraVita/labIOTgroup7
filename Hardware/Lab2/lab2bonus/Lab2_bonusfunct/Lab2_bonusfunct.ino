const int LED_PIN_VERDE = 2;
const int SOUND_PIN = 7;

int ts1 = millis(); // ultimo evento rilevato
int ts2 = millis(); // eventi multipli
int event_counter = 0;
long int n_sound_events = 50;
long int n_sound_events_battito = 25;
long int sound_interval = 600*1e3; //10 minuti (600)
long int sound_interval_battito = 4*1e3;
long int timeout_sound = 6*1e3; //60 minuti (3600)
bool statoOn=0;
  
void statoVerdeChange()
{
  
  if(statoOn==0){
    digitalWrite(LED_PIN_VERDE,HIGH);
    statoOn=1;
  }
  else{
    digitalWrite(LED_PIN_VERDE,LOW);
    statoOn=0;
    
  }
}

void battitoMani(){
  if(event_counter == 0)
    ts1 = millis();
  event_counter++;  
  Serial.print("Event "); Serial.println(event_counter);
  ts2 = millis();
  // Serial.print("Event "); Serial.println(event_counter);
  if( (ts2 - ts1) <= sound_interval_battito && event_counter >= n_sound_events_battito)  {
    statoVerdeChange(); //se ci sono c'è qualcuno
    event_counter = 0;
  }
  else if ((ts2 - ts1) > sound_interval_battito)
    event_counter = 0; //se il timer supera i 10 minuti il counter torna a 0
  
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 2 Starting...");
  
  
  
  pinMode(LED_PIN_VERDE, OUTPUT);

  
  attachInterrupt(digitalPinToInterrupt(SOUND_PIN), battitoMani, FALLING);
}

void loop() {
  //Serial.println();
  //Serial.print("ts1 "); Serial.println(ts1);
  
  

}
