import os
import threading
import socket
class EspiaClient(threading.Thread):
    def __init__(self, connection, client_info, controller, server):
        threading.Thread.__init__(self)
        self.connection = connection        # Connection socket
        self.client_info = client_info      # Client infomation
        self.paused = True                  # To indicate pausing of the current thread (implemented to break the while loop in run())
        self.closed = False                 # To indicate closing of the current thread (implemented to break the while loop in run())
        self.controller = controller        # Controller condition object (to awake, sleep it)
        self.state = threading.Condition()  # Current thread condition object (used to awake, sleep this thread)
        self.server = server                # Server object
        self.recieving = True               # To busy-wait while recieving something
        self.final = ""                     # Buffer to add recieving data

    def run(self):
        self.recv_thread = threading.Thread(name='Reciever', target=self.recieve)
        self.recv_thread.start()

        self.pause()
        while (self.paused == False and self.closed == False):
            self.recieving = True
            self.final = ""
            input_text = input("%s:%s >> " % (self.client_info[0], self.client_info[1]))
            parameters = input_text.split(" ")
            command = parameters[0]
            parameters = parameters[1:]

            if (command == "ls"):
                self.connection.send("ls".encode())
            elif (command == "pwd"):
                self.connection.send("pwd".encode())
            elif (command == "cd"):
                self.connection.send(input_text.encode())
            elif (command == "whoami"):
                self.connection.send(command.encode())
            elif (command == "exit"):
                self.close() 
            elif (command == "wait"):
                self.pause()
            else:
                continue
            
            while (self.recieving):
                pass
            print(self.final)
        return
    
    # Seperate recieve thread to recieve data from the client (Make it mutually exlusive)
    def recieve(self):
        while True:
            try:
                recv_buff = self.connection.recv(1024).decode()
                if (recv_buff[-20::2][:5] == "<end>"):              # For some reason, '<end>' that is in recv_buff will be actually ['<','','e','','n','','d','','>'] so normal comparison won't work, so those '' need to be removed
                    self.final += recv_buff[0:-20]
                    self.recieving = False
                else:
                    self.final += recv_buff
            except ConnectionResetError:
                self.close()
            except OSError:
                return

    # Closes the connection socket, clears thread data
    def close(self):
        self.recieving = False
        self.closed = True
        try:
            self.connection.shutdown(socket.SHUT_RDWR)    # Try closing the both ends of the connection to exit from recieve() function thread
        except OSError:
            pass
        self.connection.close()                         # Close the socket
        self.server.remove_client(self)                 # Remove itself from thread pool
        self.notify_controller()                        # Notify the controller to take control
        print("[*] %s:%s got disconnected" % (self.client_info[0], self.client_info[1]))
        if self.paused == True:                         # If already paused, continue (This causes the while loop in run() to break)
            self.resume()

    # Pauses the current thread (Awake the controller and pause)
    def pause(self):
        self.recieving = False
        with self.state:
            with self.controller:                      # Notify the controller to awake
                self.controller.notify()
            self.paused = True
            self.state.wait()                           # Pause the current thread
    
    # Resumes the current thread (Take control after controller is paused)
    def resume(self):
        with self.state:
            self.paused = False
            self.state.notify()                         # Notify  itself to awake
    
    # Notify the controller to take back control (Awake the controller)
    def notify_controller(self):
        with self.controller:
            self.controller.notify()