import cherrypy, json, os, time, threading

threadLock = threading.Lock()
flag = True

class Check(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)


    def run(self):
        global flag
        global threadLock
        while flag:
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
            time.sleep(60)
        print("Thread stopped.")

@cherrypy.expose
class Broker:
    def GET(self, *uri, **params):
        with open(f"{os.getcwd()}/res/data.json") as fp:
            dict = json.load(fp)
        broker = dict["broker"]
        return json.dumps(broker, indent=4)

@cherrypy.expose
class Devices:
    def GET(self, *uri, **params):
        with open(f"{os.getcwd()}/res/data.json") as fp:
            dict = json.load(fp)

        if len(uri) == 0:
            devices = dict["devices"]
            return json.dumps(devices, indent=4)

        elif len(uri) == 1:
            devices = dict["devices"]
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
            dict = json.load(fp)

        if any(item for item in dict["devices"] if item["deviceID"] == body["deviceID"]):
            raise cherrypy.HTTPError(409, "DeviceID already exists.")

        body["insert-timestamp"] = time.time()

        dict["devices"].append(body)

        threadLock.acquire()
        with open(f"{os.getcwd()}/res/data.json", "w") as fp:
            fp.write(json.dumps(dict, indent=4))
        threadLock.release()

@cherrypy.expose
class Users:
    def GET(self, *uri, **params):
        with open(f"{os.getcwd()}/res/data.json") as fp:
            dict = json.load(fp)

        if len(uri) == 0:
            users = dict["users"]
            return json.dumps(users, indent=4)

        elif len(uri) == 1:
            users = dict["users"]
            userID = uri[0]
            try:
                user = next(item for item in users if item["userID"] == userID) #next restituisce il primo oggetto trovato. Per una lista usare filter()
            except StopIteration:
                raise cherrypy.HTTPError(404, "UserID does not exists.")
            return json.dumps(user, indent=4)

        else:
            raise cherrypy.HTTPError(400, "Bad Request")

    def POST(self, *uri, **params):
        global threadLock
        body = json.loads(cherrypy.request.body.read())

        with open(f"{os.getcwd()}/res/data.json") as fp:
            dict = json.load(fp)

        if any(item for item in dict["users"] if item["userID"] == body["userID"]):
            raise cherrypy.HTTPError(409, "UserID already exists.")

        dict["users"].append(json.loads(body))

        threadLock.acquire()
        with open(f"{os.getcwd()}/res/data.json", "w") as fp:
            fp.write(json.dumps(dict, indent=4))
        threadLock.release()

@cherrypy.expose
class Services:
    def GET(self, *uri, **params):
        with open(f"{os.getcwd()}/res/data.json") as fp:
            dict = json.load(fp)

        if len(uri) == 0:
            services = dict["services"]
            return json.dumps(services, indent=4)

        elif len(uri) == 1:
            services = dict["services"]
            serviceID = uri[0]
            service = next(item for item in services if item["serviceID"] == serviceID) #next restituisce il primo oggetto trovato. Per una lista usare filter()
            return json.dumps(service, indent=4)

        else:
            raise cherrypy.HTTPError(409, "Bad Request")

    def POST(self, *uri, **params):
        body = json.loads(cherrypy.request.body.read())
        with open(f"{os.getcwd()}/res/data.json") as fp:
            dict = json.load(fp)

        if any(item for item in dict["service"] if item["serviceID"] == body["serviceID"]):
            raise cherrypy.HTTPError(409, "ServiceID already exists.")

        body["insert-timestamp"] = time.time()

        dict["services"].append(json.loads(body))

        threadLock.acquire()
        with open(f"{os.getcwd()}/res/data.json", "w") as fp:
            fp.write(json.dumps(dict, indent=4))
        threadLock.release()

@cherrypy.expose
class Refresh:
    def GET(self, *uri, **params):

        with open(f"{os.getcwd()}/res/data.json") as fp:
            dict = json.load(fp)

        if len(uri) == 0:

            for deviceIndex in range(len(dict["devices"])):
                dict["devices"][deviceIndex]["insert-timestamp"] = time.time()

            threadLock.acquire()
            with open(f"{os.getcwd()}/res/data.json", "w") as fp:
                fp.write(json.dumps(dict, indent=4))
            threadLock.release()

        elif len(uri) == 1:

            found = False
            for deviceIndex, device in enumerate(dict["devices"]):
                if device["deviceID"] == uri[0]:
                    dict["devices"][deviceIndex]["insert-timestamp"] = time.time()
                    found = True
                    break
            if found == False:
                raise cherrypy.HTTPError(404, "Device to refresh not found.")


            threadLock.acquire()
            with open(f"{os.getcwd()}/res/data.json", "w") as fp:
                fp.write(json.dumps(dict, indent=4))
            threadLock.release()

        else:
            raise cherrypy.HTTPError(400, "Bad Request.")

@cherrypy.expose
class Registration:
    def POST(self, *uri, **params):
        body = json.loads(cherrypy.request.body.read())
        with open(f"{os.getcwd()}/res/data.json") as fp:
            dict = json.load(fp)

        device = body["device"]
        found = False
        for i, item in enumerate(dict["devices"]):
            if item["deviceID"]==device["deviceID"]:
                item = device
                item["insert-timestamp"] = time.time()
                dict["devices"][i]=item
                print("Updated device ", item["deviceID"])
                found = True
                break
        if not found:
            body["device"]["insert-timestamp"] = time.time()
            dict["devices"].append(body["device"])


        for i, service in enumerate(body["services"]):
            found = False
            for item in dict["services"]:
                if item["serviceID"]==service["serviceID"]:
                    item = service
                    item["insert-timestamp"] = time.time()
                    dict["services"][i]=item
                    print("Updated service ", item["serviceID"])
                    found = True
                    break
            if not found:
                service["insert-timestamp"] = time.time()
                dict["services"].append(service)

        # print(json.dumps(dict, indent=4))

        threadLock.acquire()
        with open(f"{os.getcwd()}/res/data.json", "w") as fp:
            fp.write(json.dumps(dict, indent=4))
        threadLock.release()

if __name__ == "__main__":
    conf = {
        "/": {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
        }
    }


    thd = Check()
    thd.start()
    cherrypy.tree.mount(Broker(), "/broker", conf)
    cherrypy.tree.mount(Devices(), "/devices", conf)
    cherrypy.tree.mount(Users(), "/users", conf)
    cherrypy.tree.mount(Services(), "/services", conf)
    cherrypy.tree.mount(Refresh(), "/refresh", conf)
    cherrypy.tree.mount(Registration(), "/registration", conf)
    #cherrypy.config.update({'server.socket_host': '0.0.0.0'})
    try:
        cherrypy.engine.start()
        cherrypy.engine.block()
    except KeyboardInterrupt:
        flag = False
