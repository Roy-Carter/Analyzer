import socket
import os
LONG = 4
BUFFER = 1024
SPACE = " "


def send_file(conn, name):
    flag = False
    try:
        full_path = "Output/"+name
        file_to_send = open(full_path, "rb")
        size = os.path.getsize(full_path)
        file_name = name + "\n"
        size_to_send = str(size) + "\n"
        conn.send(size_to_send.encode())
        conn.send(file_name.encode())

        while size > 0:
            data = file_to_send.read(1024)
            conn.send(data)
            size -= len(data)

        file_to_send.close()
        flag = True

    except IOError:
        print("FILE IS ALREADY OPEN")
    finally:
        return flag


def client_program():
    host = socket.gethostname()  # as both code is running on same pc
    port = 4444 # socket server port number
    client_socket = socket.socket()  # instantiate
    client_socket.connect(("127.0.0.1", port))  # connect to the server
    flag = send_file(client_socket, "log.txt")
    client_socket.shutdown(2)
    client_socket.close()
    if flag:
        client_socket1 = socket.socket()  # instantiate
        client_socket1.connect(("127.0.0.1", 4445))
        send_file(client_socket1, "p_names.txt")
        client_socket1.shutdown(2)
        client_socket1.close()




