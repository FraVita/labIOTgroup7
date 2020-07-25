import paho.mqtt.client as PahoMQTT
import json, time, random, string, requests

server_ip = "http://localhost:8080" #inserire inidirzzo IP del server

class Publisher:
    def __init__(self, clientID, topic, broker):
        self._paho_mqtt = PahoMQTT.Client(clientID, clean_session= True)
        self.messageBroker = broker
        self.topic = topic
        self._paho_mqtt.on_connect = self.onConnect

    def onConnect(self, paho_mqtt, userdata, flags, rc):
        print(f"Connected to {self.messageBroker} with result code: {rc}")

    def start(self):
        self._paho_mqtt.connect(self.messageBroker, port=1883)
        self._paho_mqtt.loop_start()

    def stop(self):
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()

    def myPublish(self, message):
        self._paho_mqtt.publish(self.topic, message, 2)
        print(f"Published at topic: {self.topic}\n{message}")

if __name__ == "__main__":

    # clientID = "".join(random.choices(string.ascii_letters + string.digits, k=5)) #genera un ID random
    clientID = "catalog_IoT_Tech"

    topic = "/%s/refresh/or/add/device" %clientID
    broker = "mqtt.eclipse.org"
    publisherID = clientID+"p"
    pub = Publisher(publisherID, topic, broker)
    pub.start()
    time.sleep(1)
    while True:

        instructions = {}
        num = random.randrange(1000)
        print(num)

        res = ("temperature", "humidity", "motion-sensor")
        if num%2 == 0:
            if num%10 == 0:
                instructions["command"] = "refresh"
                instructions["body"] = {
                    "deviceID": "all"
                }
            else:
                r = requests.get(server_ip + "/devices")
                list = r.json()
                if len(list) == 0:
                    instructions["command"] = "add_device"
                    instructions["body"] = {
                        "deviceID": "".join(random.choices(string.ascii_letters + string.digits, k=5)),
                        "endpoints": "provaMQTT",
                        "resources": random.choices(res, k=random.randrange(1,3))
                    }
                else:
                    device = random.choice(list) #choice restituisce un elemento, choices una lista.
                    deviceID = device["deviceID"]
                    instructions["command"] = "refresh"
                    instructions["body"] = {
                        "deviceID": deviceID
                    }

        else:
            instructions["command"] = "add_device"
            instructions["body"] = {
                "deviceID": "".join(random.choices(string.ascii_letters + string.digits, k=5)),
                "endpoints": "provaMQTT",
                "resources": random.choices(res, k=random.randrange(1,3))
            }

        pub.myPublish(json.dumps(instructions))
        time.sleep(60)

    pub.stop()
    print("Publisher disconnected.")
