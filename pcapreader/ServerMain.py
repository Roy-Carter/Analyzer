from Server import *


def main():
    server = Server(HOST, PORT)
    ret_val = server.handle_server()


if __name__ == '__main__':
    main()
