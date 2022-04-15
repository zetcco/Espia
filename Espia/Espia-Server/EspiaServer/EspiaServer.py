import socket
import threading
import EspiaClient
import EspiaErrors
class EspiaServer(threading.Thread):
    server_ip = None 
    server_port = None
    isAccepting = True
    clients = []

    def __init__(self, controller, server_ip = "", server_port = 5555):
        self.server_ip = server_ip
        self.server_port = server_port
        self.controller = controller
        self.condition = threading.Condition()
        threading.Thread.__init__(self)

    def run(self):
        try:
            self.connection_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.connection_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.connection_socket.bind((self.server_ip, self.server_port))
            self.connection_socket.listen()
            print("[*] Started listening on %s" % (self.server_port))
            while self.isAccepting:
                connection, client_info = self.connection_socket.accept()
                if (client_info[0] != "127.0.0.1"):
                    print("[*] Got connection %s:%s" % (client_info[0], client_info[1]))
                    new_client = EspiaClient.EspiaClient(connection, client_info, self.controller, self)
                    new_client.start()
                    self.clients.append(new_client)
            
            print("[*] Stopped listening on %s" % (self.server_port))
            return
        except socket.error as err:
            print("Failed to initialize the server : %s" % (err))
    
    def resume_client(self, index):
        if (len(self.clients) == 0):
            raise EspiaErrors.NoClientsConnected
        self.clients[index].resume()
    
    def list_clients(self):
        for i in self.clients:
            print("%d - %s:%s" % (self.clients.index(i), i.client_info[0], i.client_info[1]))
    
    def remove_client(self, client):
        self.clients.remove(client)
    
    def shutdown(self):
        self.isAccepting = False
        temp_clients = self.clients[:]
        for client in temp_clients:
            client.close()

        socket.socket(socket.AF_INET, socket.SOCK_STREAM).connect((self.server_ip, self.server_port))
        self.connection_socket.close()