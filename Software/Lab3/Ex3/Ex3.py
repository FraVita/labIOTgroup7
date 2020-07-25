import paho.mqtt.client as PahoMQTT
import json, requests, random, string, time, threading

server_ip = "http://localhost:8080"
door = False

class RefreshService(threading.Thread):
    global door
    def __init__(self, clientID):
        threading.Thread.__init__(self)
        self.clientID = clientID
    def run(self):
        while True:
            for i in range(60):
                time.sleep(1)
                if door:
                    break
            if door:
                break

            r = requests.get(server_ip + "/refresh/" + self.clientID)
            print("Refreshed.")



class Publisher:
    def __init__(self, clientID, broker, topic, port=1883):
        self.messageBroker = broker
        self.port = port
        self._paho_mqtt = PahoMQTT.Client(clientID, clean_session= True)
        self.topic = topic
        self._paho_mqtt.on_connect = self.onConnect
        self._paho_mqtt.on_publish = self.myPublish

    def start(self):
        self._paho_mqtt.connect(self.messageBroker, port=self.port)
        self._paho_mqtt.loop_start()
        self._paho_mqtt.subscribe(self.topic, 2)
        print("Subscribed at topic %s" %self.topic)

    def stop(self):
        self._paho_mqtt.unsubscribe(self.topic)
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()

    def onConnect(self, paho_mqtt, userdata, flags, rc):
        print(f"Connected to {self.messageBroker} with result code: {rc}")

    def myPublish(self, message):
        self._paho_mqtt.publish(self.topic, message, 2)
        print(f"Published at topic: {self.topic}\n{message}")

if __name__ == "__main__":

    # recuperiamo il broker
    r = requests.get(server_ip + "/broker")
    if r.status_code >= 400:
        print("Broker - HTTP Error: %s" %r.status_code)
        exit()

    else:
        broker = r.json()
        address = broker["address"]
        port = broker["port"]

    #recuperiamo il topic dell'arduino
    r = requests.get(server_ip + "/devices/Yun-IoT_Tech-Group7")
    if r.status_code >= 400:
        print("Device - HTTP Error: %s" %r.status_code)
        exit()
    else:
        device = r.json()
        topics = device["endpoints"]

        publishers = []
        for topic in topics:
            ranID = "".join(random.choices(string.ascii_letters + string.digits, k=5))
            clientID = "service_"+ranID
            sub = Publisher(clientID, address, topic, port)
            publishers.append(sub)

        for publisher in publishers:
            publisher.start()

    # aggiungiamo il servizio al catalogo
    jDict = {}
    jDict["serviceID"] = clientID
    jDict["description"] = "Led control service for Arduino Yun"
    jDict["endpoints"] = topics

    message = json.dumps(jDict)
    r = requests.post(server_ip+"/services", data= message)
    time.sleep(1)
    thd = RefreshService(clientID)
    thd.start()

    try:
        if r.status_code >= 400:
            print("Service - HTTP Error: %s" %r.status_code)
            exit()

        ledOn = True #DA CONTROLLARE CON ARDUINO (true o 1)
        while True:
            jLedDict = {
                "bn": "Yun-IoT_Tech-Group7",
                "e": [{
                    "n": "led",
                    "t": time.time(),
                    "v": ledOn,
                    "u": "null"
                    }
                ]
            }

            for publisher in publishers:
                publisher.myPublish(message=json.dumps(jLedDict))
            ledOn = not ledOn
            time.sleep(10)

    except KeyboardInterrupt:
        print("\nProgram Stopped")

    finally:
        door = True
        for publisher in publishers:
            publisher.stop()
        print("Publishers stopped")
