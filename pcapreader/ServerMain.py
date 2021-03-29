from Server import *
from ConnectToGUI import *


def main():
    server = Server(HOST, PORT)
    ret_val = server.handle_server()
    client_program() # check about the i/o problem


if __name__ == '__main__':
    main()
