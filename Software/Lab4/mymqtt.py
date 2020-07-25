import paho.mqtt.client as PahoMQTT
import json, requests

class Publisher:
    def __init__(self, clientID, broker, port= 1883):
        self.messageBroker = broker
        self.port = port
        self.__paho_mqtt = PahoMQTT.Client(clientID, clean_session=True)
        self.__paho_mqtt.on_connect = self.onConnect
        self.__paho_mqtt.on_publish = self.onPublish

    def start(self):
        self.__paho_mqtt.connect(self.messageBroker, port= self.port)
        self.__paho_mqtt.loop_start()

    def stop(self):
        self.__paho_mqtt.loop_stop()
        self.__paho_mqtt.disconnect()

    def onConnect(self, paho_mqtt, userdata, flags, rc):
        print(f"Publisher connected to {self.messageBroker} with result code: {rc}")

    def onPublish(self, client, userdata, result):
        print("MQTT - Data published.")

    def myPublish(self, topic, message):
        res = self.__paho_mqtt.publish(topic, message, 2)
        return res

class Subscriber:
    def __init__(self, clientID, broker, topic, port = 1883):
        self.messageBroker = broker
        self.port = port
        self.topic = topic
        self.__paho_mqtt = PahoMQTT.Client(clientID, clean_session=True)
        self.__paho_mqtt.on_connect = self.onConnect
        self.__paho_mqtt.on_message = self.onMessage

    def start(self):
        self.__paho_mqtt.connect(self.messageBroker, port = self.port)
        self.__paho_mqtt.subscribe(self.topic, 2)
        self.__paho_mqtt.loop_start()

    def stop(self):
        self.__paho_mqtt.loop_stop()
        self.__paho_mqtt.unsubscribe(self.topic)
        self.__paho_mqtt.disconnect()

    def onConnect(self, paho_mqtt, userdata, flags, rc):
        print(f"Subscriber connected to {self.messageBroker} with result code: {rc}")
        print(f"Topic: {self.topic}")

    def onMessage(self, client, userdata, message):
        jPayload = json.loads(message.payload)

        jDict = {
            "deviceID": jPayload["bn"] + "_" + jPayload["e"][0]["n"],
            "resources": [
                "temperature"
                ],
            "values": [
                jPayload["e"][0]["v"]
                ],
            "units": [
                jPayload["e"][0]["u"]
                ]
        }

        # print(json.dumps(jDict,indent=4))

        print("PUT UPDATE")
        r = requests.put("http://localhost:8080/catalog/update", data=json.dumps(jDict))
        if r.status_code == 404:
            re = requests.post("http://localhost:8080/catalog/register", data=json.dumps(jDict))
            print("POST REGISTER")
            if re.status_code != 200:
                print("HTTP Error: %s" %re.status_code)
        elif r.status_code != 200:
            print("HTTP Error: %s" %r.status_code)
