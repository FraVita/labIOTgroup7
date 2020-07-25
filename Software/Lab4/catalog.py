import cherrypy, json, os, time, mymqtt

def register(new_device):
    new_device["insert-timestamp"] = time.time()
    with open(f"{os.getcwd()}/res/json_db.json") as f:
        data = json.loads(f.read())
    for device in data["devices"]:
        if new_device["deviceID"] in device["deviceID"]:
            return 409
    data["devices"].append(new_device)

    with open(f"{os.getcwd()}/res/json_db.json", "w") as f:
        f.write(json.dumps(data, indent=4))

    return 200

def updateLed(device_to_update):
    global publisher
    device_to_update["insert-timestamp"] = time.time()
    found = False

    with open(f"{os.getcwd()}/res/json_db.json") as f:
        data = json.loads(f.read())

    ID = device_to_update["deviceID"]

    if ID != "All":
        for ind, device in enumerate(data["devices"]):
            if device["deviceID"] == ID:
                data["devices"][ind] = device_to_update
                found = True
                break

        if not found:
            return 404
    else:
        for ind, device in enumerate(data["devices"]):
            if "illumination" in device["resources"]:
                data["devices"][ind]["values"][0] = device_to_update["values"][0]


    topic = "iot/domotic/control/service/svvgfs/light"
    jDict = {
        "bn": "Yun",
        "e": [{
            "n": ID,
            "t": time.time(),
            "v": device_to_update["values"][0],
            "u": None
        }]

    }
    message = json.dumps(jDict)
    res, mid = publisher.myPublish(topic, message)
    if res != 0:
        return 502

    with open(f"{os.getcwd()}/res/json_db.json", "w") as f:
        f.write(json.dumps(data, indent= 4))

    return 200

def updateTemp(device_to_update):
    global publisher
    device_to_update["insert-timestamp"] = time.time()
    found = False

    with open(f"{os.getcwd()}/res/json_db.json") as f:
        data = json.loads(f.read())

    ID = device_to_update["deviceID"]


    for ind, device in enumerate(data["devices"]):
        if device["deviceID"] == ID:
            data["devices"][ind] = device_to_update
            found = True
            break
    
    if not found:
        return 404

    with open(f"{os.getcwd()}/res/json_db.json", "w") as f:
        f.write(json.dumps(data, indent= 4))

    return 200

@cherrypy.expose
class Catalog:

    def POST(self, *uri, **params):
        if uri[0] == "register":
            new_device = json.loads(cherrypy.request.body.read())
            response = register(new_device)
            if response != 200:
                raise cherrypy.HTTPError(response)
        else:
            raise cherrypy.HTTPError(400, "Bad Request.")

    def PUT(self, *uri, **params):
        if uri[0] == "update":
            device_to_update = json.loads(cherrypy.request.body.read())
            print("DEVICE TO UPDATE: ", json.dumps(device_to_update))
            if "illumination" in device_to_update["resources"]:
                print("ILLUMINATION!")
                response = updateLed(device_to_update)
                if response != 200:
                    raise cherrypy.HTTPError(response)
            elif "temperature" in device_to_update["resources"]:
                print("TEMPERATURE!")
                response = updateTemp(device_to_update)
                if response != 200:
                    raise cherrypy.HTTPError(response)
        else:
            raise cherrypy.HTTPError(400, "Bad Request.")

    def GET(self, *uri, **params):
        if uri[0] == "devices" and len(uri) == 2:
            ID = uri[1]

            with open(f"{os.getcwd()}/res/json_db.json") as f:
                data = f.read()

            if ID == "all":
                return data

            else:
                found = False
                for device in json.loads(data)["devices"]:
                    if device["deviceID"] == ID:
                        found = True
                        break

                if not found:
                    raise cherrypy.HTTPError(404)

                return json.dumps(device)


class WebController:

    @cherrypy.expose
    def index(self):
        with open(f"{os.getcwd()}/web_controller/iotTech/index.html") as f:
            output = f.read()
        return output

    @cherrypy.expose
    def manage(self):
        with open(f"{os.getcwd()}/web_controller/iotTech/manage.html") as f:
            output = f.read()
        return output




def start():
    catalog_conf = {
        "/": {"request.dispatch": cherrypy.dispatch.MethodDispatcher()}
    }

    web_conf = {
        "/": {
            "tools.staticdir.root": os.path.abspath(os.getcwd())
        } ,
        "/static": {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': './web_controller/iotTech/public'
        }
    }

    cherrypy.config.update({"server.socket_host":"0.0.0.0", "server.socket_port":8080})
    cherrypy.tree.mount(Catalog(), "/catalog", catalog_conf)
    cherrypy.tree.mount(WebController(), "/", web_conf)
    cherrypy.engine.start()
    cherrypy.engine.block()

if __name__ == "__main__":
    publisher = mymqtt.Publisher("catalog_pub", broker="mqtt.eclipse.org")
    publisher.start()

    subscriber = mymqtt.Subscriber("catalog_sub", broker="mqtt.eclipse.org", topic="iot/domotic/control/service/svvgfs/sensors/+")
    subscriber.start()

    start()
