import threading
import socket
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
        self.recv_thread = threading.Thread(name='Reciever', target=self.recieve)
        self.recv_thread.start()

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
            elif (command == "wait"):
                self.pause()

        return
    
    # Seperate recieve thread to recieve data from the client (Make it mutually exlusive)
    def recieve(self):
        try:
            recv_buff = self.connection.recv(1024)
            if (len(recv_buff) != 0):
                recv_buff
        except ConnectionResetError:
            self.close()

    # Closes the connection socket, clears thread data
    def close(self):
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