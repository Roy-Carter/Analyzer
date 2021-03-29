import socket
import os
LONG = 4
BUFFER = 1024


def send_file(conn):
    try:
        file_to_send = open("Output/log.txt", "rb")
        size = os.path.getsize("Output/log.txt")
        file_name = "log.txt" + "\n"
        conn.send(file_name.encode())

        while size > 0:
            data = file_to_send.read(1024)
            #print("Sending...")
            conn.send(data)
            size -= len(data)

        file_to_send.close()
        #print("Done Sending.")
        conn.shutdown(2)
        conn.close()
    except IOError:
        print("FILE IS ALREADY OPEN")


def client_program():
    host = socket.gethostname()  # as both code is running on same pc
    port = 4444 # socket server port number
    client_socket = socket.socket()  # instantiate
    client_socket.connect(("127.0.0.1", port))  # connect to the server
    send_file(client_socket)


