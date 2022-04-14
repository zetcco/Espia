import threading
class EspiaClient(threading.Thread):
    def __init__(self, connection, client_info, controller):
        self.connection = connection
        self.client_info = client_info
        threading.Thread.__init__(self)
        self.state = threading.Condition()
        self.controller = controller

    def run(self):
        self.pause()
        while True:
            command = input("%s >> " % (self.client_info[0]))
            if (command == "exit"):
                self.connection.close()
                print("[*] %s got disconnected" % (self.client_info[0]))
                with self.controller:
                    self.controller.notify()
                break
            elif (command == "wait"):
                self.pause()

    def pause(self):
        with self.state:
            with self.controller:
                self.controller.notify()
            self.state.wait()
    
    def resume(self):
        with self.state:
            self.state.notify()