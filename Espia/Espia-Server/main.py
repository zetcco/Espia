from EspiaServer import EspiaServer
import threading

def control(cond):
    while True:
        test = input(">> ")

        if (test == "nolisten"):
            client.isAccepting = False
        elif (test == "list"):
            for i in client.clients:
                print(i.client_info[0])
        elif (test == "select"):
            client.clients[0].resume()
            with cond:
                cond.wait()

condition = threading.Condition()

client = EspiaServer.EspiaServer(condition, server_port=8888)
client.start()

c1 = threading.Thread(name='control', target=control, args=(condition,))
c1.start()