import cherrypy, json, os, time, threading, random, string
import paho.mqtt.client as PahoMQTT

threadLock = threading.Lock()
exit = False
server_ip = "http://localhost:8080"

class Subscriber:
    def __init__(self, clientID, broker, port=1883):
        self.messageBroker = broker
        self.port = port
        self._paho_mqtt = PahoMQTT.Client(clientID, clean_session= True)
        self.topic = "/%s/refresh/or/add/device" %clientID
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
        global threadLock

        instructions = json.loads(message.payload)
        if instructions["command"] == "refresh":

            with open(f"{os.getcwd()}/res/data.json") as fp:
                jDict = json.load(fp)

            if instructions["body"]["deviceID"] == "all":
                for device in jDict["devices"]:
                    device["insert-timestamp"] = time.time()

                threadLock.acquire()
                with open(f"{os.getcwd()}/res/data.json", "w") as fp:
                    fp.write(json.dumps(jDict, indent=4))
                threadLock.release()

            else:
                found = False
                for device in jDict["devices"]:
                    if device["deviceID"] == instructions["body"]["deviceID"]:
                        device["insert-timestamp"] = time.time()
                        found = True
                        break
                if found == False:
                    print("Device not found.")


                threadLock.acquire()
                with open(f"{os.getcwd()}/res/data.json", "w") as fp:
                    fp.write(json.dumps(jDict, indent=4))
                threadLock.release()

        elif instructions["command"] == "add_device":

            with open(f"{os.getcwd()}/res/data.json") as fp:
                jDict = json.load(fp)

            if any(item for item in jDict["devices"] if item["deviceID"] == instructions["body"]["deviceID"]):
                print("Device already exists.")

            instructions["body"]["insert-timestamp"] = time.time()

            jDict["devices"].append(instructions["body"])

            threadLock.acquire()
            with open(f"{os.getcwd()}/res/data.json", "w") as fp:
                fp.write(json.dumps(jDict, indent=4))
            threadLock.release()

class Check(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        global exit
        global threadLock
        while True:
            for i in range(60):
                time.sleep(1)
                if exit:
                    break
            if exit:
                break
            threadLock.acquire()

            with open(f"{os.getcwd()}/res/data.json") as fp:
                data = json.load(fp)

            for device in data["devices"]:
                if time.time() - device["insert-timestamp"] > 120:
                    data["devices"].remove(device)
                    print(f'Deleted device: {device["deviceID"]}')

            for service in data["services"]:
                if time.time() - service["insert-timestamp"] > 120:
                    data["services"].remove(service)
                    print(f'Deleted service: {service["serviceID"]}')

            with open(f"{os.getcwd()}/res/data.json", "w") as fp:
                fp.write(json.dumps(data, indent=4))
            threadLock.release()

        print("Thread stopped.")

@cherrypy.expose
class Broker:
    def GET(self, *uri, **params):
        with open(f"{os.getcwd()}/res/data.json") as fp:
            jDict = json.load(fp)
        broker = jDict["broker"]
        return json.dumps(broker, indent=4)

@cherrypy.expose
class Devices:
    def GET(self, *uri, **params):
        with open(f"{os.getcwd()}/res/data.json") as fp:
            jDict = json.load(fp)

        if len(uri) == 0:
            devices = jDict["devices"]
            return json.dumps(devices, indent=4)

        elif len(uri) == 1:
            devices = jDict["devices"]
            deviceID = uri[0]
            try:
                device = next(item for item in devices if item["deviceID"] == deviceID) #next restituisce il primo oggetto trovato. Per una lista usare filter()
            except StopIteration:
                raise cherrypy.HTTPError(404, "DeviceID does not exists")
            return json.dumps(device, indent=4)

        else:
            raise cherrypy.HTTPError(400, "Bad Request")

    def POST(self, *uri, **params):
        global threadLock
        body = json.loads(cherrypy.request.body.read())

        with open(f"{os.getcwd()}/res/data.json") as fp:
            jDict = json.load(fp)

        if any(item for item in jDict["devices"] if item["deviceID"] == body["deviceID"]):
            raise cherrypy.HTTPError(409, "DeviceID already exists.")

        body["insert-timestamp"] = time.time()

        jDict["devices"].append(body)

        threadLock.acquire()
        with open(f"{os.getcwd()}/res/data.json", "w") as fp:
            fp.write(json.dumps(jDict, indent=4))
        threadLock.release()

@cherrypy.expose
class Users:
    def GET(self, *uri, **params):
        with open(f"{os.getcwd()}/res/data.json") as fp:
            jDict = json.load(fp)

        if len(uri) == 0:
            users = jDict["users"]
            return json.dumps(users, indent=4)

        elif len(uri) == 1:
            users = jDict["users"]
            userID = uri[0]
            try:
                user = next(item for item in users if item["userID"] == userID) #next restituisce il primo oggetto trovato. Per restituire una lista usare filter()
            except StopIteration:
                raise cherrypy.HTTPError(404, "UserID does not exists.")
            return json.dumps(user, indent=4)

        else:
            raise cherrypy.HTTPError(400, "Bad Request")

    def POST(self, *uri, **params):
        global threadLock
        body = json.loads(cherrypy.request.body.read())

        with open(f"{os.getcwd()}/res/data.json") as fp:
            jDict = json.load(fp)

        if any(item for item in jDict["users"] if item["userID"] == body["userID"]):
            raise cherrypy.HTTPError(409, "UserID already exists.")

        jDict["users"].append(body)

        threadLock.acquire()
        with open(f"{os.getcwd()}/res/data.json", "w") as fp:
            fp.write(json.dumps(jDict, indent=4))
        threadLock.release()

@cherrypy.expose
class Services:
    def GET(self, *uri, **params):
        with open(f"{os.getcwd()}/res/data.json") as fp:
            jDict = json.load(fp)

        if len(uri) == 0:
            services = jDict["services"]
            return json.dumps(services, indent=4)

        elif len(uri) == 1:
            services = jDict["services"]
            serviceID = uri[0]
            service = next(item for item in services if item["serviceID"] == serviceID) #next restituisce il primo oggetto trovato. Per una lista usare filter()
            return json.dumps(service, indent=4)

        else:
            raise cherrypy.HTTPError(409, "Bad Request")

    def POST(self, *uri, **params):
        body = json.loads(cherrypy.request.body.read())
        with open(f"{os.getcwd()}/res/data.json") as fp:
            jDict = json.load(fp)

        if any(item for item in jDict["services"] if item["serviceID"] == body["serviceID"]):
            raise cherrypy.HTTPError(409, "ServiceID already exists.")

        body["insert-timestamp"] = time.time()

        jDict["services"].append(body)

        threadLock.acquire()
        with open(f"{os.getcwd()}/res/data.json", "w") as fp:
            fp.write(json.dumps(jDict, indent=4))
        threadLock.release()

@cherrypy.expose
class Refresh:
    def GET(self, *uri, **params):

        with open(f"{os.getcwd()}/res/data.json") as fp:
            jDict = json.load(fp)

        if len(uri) == 0:

            for device in jDict["devices"]:
                device["insert-timestamp"] = time.time()

            threadLock.acquire()
            with open(f"{os.getcwd()}/res/data.json", "w") as fp:
                fp.write(json.dumps(jDict, indent=4))
            threadLock.release()

        elif len(uri) == 1:

            found = False
            for device in jDict["devices"]:
                if device["deviceID"] == uri[0]:
                    device["insert-timestamp"] = time.time()
                    found = True
                    break
            if found == False:
                raise cherrypy.HTTPError(404, "Device to refresh not found.")


            threadLock.acquire()
            with open(f"{os.getcwd()}/res/data.json", "w") as fp:
                fp.write(json.dumps(jDict, indent=4))
            threadLock.release()

        else:
            raise cherrypy.HTTPError(400, "Bad Request.")

if __name__ == "__main__":
    conf = {
        "/": {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
        }
    }

    # clientID = "catalog" + "".join(random.choices(string.ascii_letters + string.digits, k=5)) #genera un ID random
    clientID = "catalog_IoT_Tech"
    with open(f"{os.getcwd()}/res/data.json") as fp:
        data = json.load(fp)
        broker = data["broker"]["address"]
        port = data["broker"]["port"]
    sub = Subscriber(clientID, broker, port)
    sub.start()

    thd = Check()
    thd.start()
    cherrypy.tree.mount(Broker(), "/broker", conf)
    cherrypy.tree.mount(Devices(), "/devices", conf)
    cherrypy.tree.mount(Users(), "/users", conf)
    cherrypy.tree.mount(Services(), "/services", conf)
    cherrypy.tree.mount(Refresh(), "/refresh", conf)
    #cherrypy.config.update({'server.socket_host': '0.0.0.0'})
    try:
        cherrypy.engine.start()
        cherrypy.engine.block()
    except KeyboardInterrupt:
        sub.stop()
        exit = True
