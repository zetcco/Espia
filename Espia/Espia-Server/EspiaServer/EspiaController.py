from EspiaServer import EspiaServer
import threading

class EspiaController(threading.Thread):
    def __init__(self, server_port):
        self.server_port = server_port
        self.condition = threading.Condition()
        threading.Thread.__init__(self)
    
    def run(self):
        self.start_listening()
        self.start_controller()
        return

    def start_listening(self):
        self.server_thread = EspiaServer(self.condition, server_port=self.server_port)
        self.server_thread.start()
    
    def start_controller(self):
        while True:
            command = input(">> ")

            if (command == "list"):
                self.server_thread.list_clients()
            elif (command == "select"):
                #parameters = command.split(" ")
                self.server_thread.resume_client(0)
                with self.condition:
                    self.condition.wait()
            elif (command == "exit"):
                self.server_thread.shutdown()
                return

controller = EspiaController(8888)
controller.start()