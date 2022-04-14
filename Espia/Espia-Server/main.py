from EspiaServer import EspiaServer

client = EspiaServer.EspiaServer(server_port=8888)
client.start()

while True:
    test = input(">> ")

    if (test == "exit"):
        client.isAccepting = False