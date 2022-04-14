import threading

class EspiaClient(threading.Thread):
    def __init__(self, connection, client_info):
        self.connection = connection
        self.client_info = client_info
        threading.Thread.__init__(self)
    
    def run(self):
        while True:
            command = input("%s >>" % (self.client_info[0]))
            print("Sent %s" % (command))