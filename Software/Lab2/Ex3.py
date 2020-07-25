import requests, random, string, time, json


server_ip = "http://localhost:8080" #inserire inidirzzo IP del server

def refresh(deviceID=None):
    global server_ip

    if deviceID is not None:
        endpoint = server_ip + "/refresh/" + deviceID
        r = requests.get(endpoint)
        if r.status_code >= 400:
            print(f"HTTP Error: {r.status_code}")

    else:
        endpoint = server_ip + "/refresh"
        r = requests.get(endpoint)
        if r.status_code >= 400:
            print(f"HTTP Error: {r.status_code}")

def addDevice():
    global server_ip

    endpoint = server_ip + "/devices"
    res = ("temperature", "humidity", "motion-sensor")
    device = {}
    device["deviceID"] = "".join(random.choices(string.ascii_letters + string.digits, k=5)) #genera un ID random
    device["endpoints"] = "provaMQTT"
    device["resources"] = random.choices(res, k=random.randrange(1,3))

    r = requests.post(endpoint, json.dumps(device))
    if r.status_code >= 400:
        print(f"HTTP Error: {r.status_code}")


if __name__ == "__main__":
    while True:
        num = random.randrange(1000)
        print(num)

        if num%2 == 0:
            if num%10 == 0:
                refresh()
            else:
                r = requests.get(server_ip + "/devices")
                list = r.json()
                if len(list) == 0:
                    addDevice()
                else:
                    device = random.choice(list) #choice restituisce un elemento, choices una lista.
                    deviceID = device["deviceID"]
                    refresh(deviceID)

        else:
            addDevice()

        time.sleep(60)
