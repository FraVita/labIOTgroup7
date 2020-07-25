import paho.mqtt.client as PahoMQTT
import json, requests, random, string, time


broker = "test.mosquitto.org"

class Publisher:
    def __init__(self, clientID, broker, topic, port=1883):
        self.messageBroker = broker
        self.port = port
        self.topic = topic
        self._paho_mqtt = PahoMQTT.Client(clientID, clean_session= True)
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

    clientID1 = "client_" + "".join(random.choices(string.ascii_letters + string.digits, k=5))
    topic1 = "IotTech/Test/random/number"
    publisher1 = Publisher(clientID1, broker, topic1)
    publisher1.start()

    clientID2 = "client_" + "".join(random.choices(string.ascii_letters + string.digits, k=5))
    topic2 = "IotTech/Test/random/string"
    publisher2 = Publisher(clientID2, broker, topic2)
    publisher2.start()

    try:
        while True:
            testMessage = {
                "client": clientID1,
                "number": random.randint(0, 100)
            }
            publisher1.myPublish(message=json.dumps(testMessage))
            time.sleep(5)
            testMessage = {
                "client": clientID2,
                "string": "".join(random.choices(string.ascii_letters + string.digits,
                                    k=random.randint(3, 10)))
            }
            publisher2.myPublish(message=json.dumps(testMessage))
            time.sleep(5)

    except KeyboardInterrupt:
        print("\nProgram Stopped")

    finally:
        publisher1.stop()
        print("Publisher stopped")
