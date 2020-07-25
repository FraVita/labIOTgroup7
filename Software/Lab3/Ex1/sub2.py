import paho.mqtt.client as PahoMQTT
import json, requests, random, string, time


broker = "test.mosquitto.org"

class Subscriber:
    def __init__(self, clientID, broker, topic, port=1883):
        self.messageBroker = broker
        self.port = port
        self.topic = topic
        self._paho_mqtt = PahoMQTT.Client(clientID, clean_session= True)
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
        print(f"Topic: {message.topic}\nMessage: {str(message.payload)}")


if __name__ == "__main__":

    clientID = "client_"+"".join(random.choices(string.ascii_letters + string.digits, k=5))
    topic = "IotTech/Test/random/string"

    sub = Subscriber(clientID, broker, topic)
    sub.start()
    try:
        while True:
            pass
    except KeyboardInterrupt:
        print("\nProgram Stopped")
    finally:
        sub.stop()
        print("Subscriber stopped")
