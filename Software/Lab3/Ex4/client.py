import requests, mymqtt, time, random, string

deviceID = "Yun-IoT_Tech-Group7"
url = "http://192.168.1.53:8080"

def retrieveInfos(ID = None):
    global url

    broker = requests.get(url + "/broker").json()
    address = broker["address"]
    port = broker["port"]

    if ID is not None:
        device = requests.get(url + "/devices/" + ID).json()
        endpoints = device["endpoints"]
        return address, port, endpoints
    else:
        return address, port


def start_sub():
    global deviceID

    address, port, endpoints =  retrieveInfos(deviceID)
    subs = []
    for endpoint in endpoints:
        subs.append(mymqtt.Subscriber("client_sub_L3E4"+"".join(random.choices(string.digits, k=5)), address, endpoint, port))
    for sub in subs:
        sub.start()
    return subs
def start_pub():
    global url

    address, port = retrieveInfos()

    pub = mymqtt.Publisher("client_pub_L3E4", address, port)
    pub.start()
    return pub

if __name__ == "__main__":
    subs = start_sub()
    pub = start_pub()

    try:
        while True:
            time.sleep(random.randrange(5, 10))
            pub.myPublish("/messageLCD", "".join(random.choices(string.ascii_uppercase + string.digits, k=15)))

    except KeyboardInterrupt:
        for sub in subs:
            sub.stop()
        print("Client stopped.")
