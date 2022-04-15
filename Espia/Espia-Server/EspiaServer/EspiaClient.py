import threading
class EspiaClient(threading.Thread):
    def __init__(self, connection, client_info, controller, server):
        self.connection = connection
        self.client_info = client_info
        threading.Thread.__init__(self)
        self.state = threading.Condition()
        self.paused = True
        self.closed = False
        self.controller = controller
        self.server = server

    def run(self):
        self.pause()
        while (self.paused == False and self.closed == False):
            input_text = input("%s:%s >> " % (self.client_info[0], self.client_info[1]))
            parameters = input_text.split(" ")
            command = parameters[0]
            parameters = parameters[1:]

            if (command == "ls"):
                self.connection.send("ls".encode())
            elif (command == "exit"):
                self.close() 
                self.notify_controller() 
                return
            elif (command == "wait"):
                self.pause()

        print("[*] %s:%s got disconnected" % (self.client_info[0], self.client_info[1]))
        return

    def close(self):
        self.closed = True
        self.connection.close()
        self.server.remove_client(self)
        if self.paused == True:
            self.resume()

    def pause(self):
        with self.state:
            with self.controller:
                self.controller.notify()
            self.paused = True
            self.state.wait()
    
    def resume(self):
        with self.state:
            self.paused = False
            self.state.notify()
    
    def notify_controller(self):
        with self.controller:
            self.controller.notify()