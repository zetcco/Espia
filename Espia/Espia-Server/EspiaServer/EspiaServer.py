from http import client
import socket
import threading
from EspiaServer import EspiaClient
class EspiaServer(threading.Thread):
    server_ip = None 
    server_port = None
    isAccepting = True
    clients = []

    def __init__(self, controller, server_ip = "", server_port = 5555):
        self.server_ip = server_ip
        self.server_port = server_port
        threading.Thread.__init__(self)
        self.controller = controller

    def run(self):
        try:
            self.connection_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.connection_socket.bind((self.server_ip, self.server_port))
            self.connection_socket.listen()
            while self.isAccepting:
                print("\n[*] Started listening %s" % (self.server_port))
                connection, client_info = self.connection_socket.accept()
                print("\n[*] Got connection %s:%s" % (client_info[0], client_info[1]))
                new_client = EspiaClient.EspiaClient(connection, client_info, self.controller)
                new_client.start()
                self.clients.append(new_client)
        except socket.error as err:
            print("Failed to initialize the server : %s" % (err))