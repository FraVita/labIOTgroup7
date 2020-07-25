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



class Subscriber:
    def __init__(self, clientID, broker, topic, port=1883):
        self.messageBroker = broker
        self.port = port
        self._paho_mqtt = PahoMQTT.Client(clientID, clean_session= True)
        self.topic = topic
        self._paho_mqtt.on_connect = self.onConnect
        self._paho_mqtt.on_message = self.onMessageRecived

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

    def onMessageRecived(self, client, userdata, message):
        print(json.loads(message.payload))

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

        topics =[topic for topic in device["endpoints"]]

        subscribers = []
        for topic in topics:
            ranID = "".join(random.choices(string.ascii_letters + string.digits, k=5))
            clientID = "service_"+ranID
            sub = Subscriber(clientID, address, topic, port)
            subscribers.append(sub)

        for subscriber in subscribers:
            subscriber.start()

    # aggiungiamo il servizio al catalogo
    jDict = {}
    jDict["serviceID"] = "Temp_from_yun_Service"
    jDict["description"] = "Temperature service from Arduino Yun"
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

        while True:
            time.sleep(1)

    except KeyboardInterrupt:
        print("\nProgram Stopped")

    finally:
        door = True
        for subscriber in subscribers:
            subscriber.stop()
        print("Subscribers stopped")
