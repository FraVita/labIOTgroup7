import requests, json

server_ip = "http://localhost:8080" #inserire inidirzzo IP del server
def getBroker():
    global server_ip
    endpoint = f"{server_ip}/broker"
    r = requests.get(endpoint)
    if r.status_code < 400:
        print(r.json()) #json.load(r.content)
    else:
        print(f"HTTP error: {r.status_code}")

def getDevice(deviceID):
    global server_ip
    endpoint = f"{server_ip}/devices/"

    if deviceID != "all":
        endpoint = endpoint + deviceID

    r = requests.get(endpoint)
    if r.status_code < 400:
        print(r.json()) #json.load(r.content)
    else:
        print(f"HTTP error: {r.status_code}")

def getUser(userID):
    global server_ip
    endpoint = f"{server_ip}/users/"

    if userID != "all":
        endpoint = endpoint + userID

    r = requests.get(endpoint)
    if r.status_code < 400:
        print(r.json()) #json.load(r.content)
    else:
        print(f"HTTP error: {r.status_code}")


if __name__ == "__main__":
    while True:
        print("What do you want to retrive? (broker, devices, users) Type q to exit:")
        choise = input("-> ").casefold()

        if choise == "broker":
            getBroker()

        elif choise == "devices":
            print('Insert the device ID or type "all" for all the devices.')
            choise = input("-> ")
            getDevice(choise)

        elif choise == "users":
            print('Insert the user ID or type "all" for all the users.')
            choise = input("-> ")
            getUser(choise)

        elif choise == "q":
            break

        else:
            print("Comando non valido.")
