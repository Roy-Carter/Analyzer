import socket
from LuaHandler import *
from Algo import *

# MAGIC STRINGS
BUFFER = 4096
LISTENERS = 1
LONG = 4
PCAP_NAME = "test.pcap"
NEW_PCAP_NAME = "fixed.pcap"
LUA_NAME = "descriptor.lua"
HOST = "127.0.0.1"
PORT = 2222
LISTENING_MSG = "listening at "
FILE_OPEN_ERROR = "This file name does not exists"
SEP = "=================="


class Server:
    """This class purpose is to create an object of a Server"""

    def __init__(self, host, port):
        """
        The constructor , initializes the values of the host and port and declares counter
        for both types of the protocol and initializes a dictionary that will hold the field number and
        the number of hex letters to check for that specific field
        :param host: the host ip (on local host)
        :param port: the server port
        """
        self.host = host
        self.port = port
        self.protocol_fields = {}

    def print_dict(self):
        """
        This function purpose is to print out the dictionary that descirbes the lua file
        that the server got.
        :return: None - print function
        """
        for key, value in self.protocol_fields.items():
            print(key, ' : ', value)

    @staticmethod
    def receive_file(conn, file_name):
        """
        This function is responsible of receiving a file from the C Server
        :param conn: The socket that connects between the Servers
        :param file_name: The name of the file to get the data to
        :return: Returns True if the file received correctly , False otherwise.
        """
        ret_val = False
        print("file name: " + str(file_name))
        try:
            file_object = open(file_name, "wb")
            # length of long 4 bytes
            size = conn.recv(LONG)
            # converts byte array to hex
            size = binascii.hexlify(size)
            print("length of file: " + str(size))
            # converts hexa to int
            size = int(size, 16)
            while size > 0:
                data = conn.recv(BUFFER)
                size -= len(data)
                file_object.write(data)
            file_object.close()
            ret_val = True
        except FileNotFoundError:
            print(FILE_OPEN_ERROR)
        return ret_val

    def handle_server(self):
        """
        This function is responsible of handling the server
        :return: Returns True if the server was handled correctly with no problems , otherwise
        returns False.
        """
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind((self.host, self.port))
        server.listen(LISTENERS)
        print(LISTENING_MSG + (str(server.getsockname())))
        conn, addr = server.accept()
        self.receive_file(conn, PCAP_NAME)
        self.receive_file(conn, LUA_NAME)
        lua_handler = LuaHandler()
        ret_val = lua_handler.read_lua()
        if ret_val:
            ret_val = lua_handler.parse_pcap()
            algo = MLAlgorithm()
            algo.start_algorithm()
        conn.close()
        return ret_val
