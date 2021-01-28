import socket
from LuaHandler import *
import matplotlib.pyplot as plotter


# MAGIC STRINGS
BUFFER = 4096
LISTENERS = 1
LONG = 4
PCAP_NAME = "test.pcap"
NEW_PCAP_NAME = "fixed.pcap"
LUA_NAME = "descriptor.lua"
HOST = "127.0.0.1"
PORT = 2222
TYPE_ERROR = "Error in Type"
FLAG_ERROR = "Error in Flag"
SEQ_ERROR = "Error in Seq"
PDU_ERROR = "Error in Pdu"
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
        self.request_counter = 0
        self.response_counter = 0

    def print_dict(self):
        """
        This function purpose is to print out the dictionary that descirbes the lua file
        that the server got.
        :return: None - print function
        """
        for key, value in self.protocol_fields.items():
            print(key, ' : ', value)


    def new_pcap_descriptor(self, fixed_pcap_list):
        """
        This function is responsible of creating the new pcap and printing out the number of
        request / response packets it got .
        :param fixed_pcap_list: list filled with the packets that are correct from the original pcap
        :return: None - new pcap creation and print function
        """
        wrpcap(NEW_PCAP_NAME, fixed_pcap_list)
        print(SEP)
        print("number of request packets :" + str(self.request_counter))
        print("number of response packets :" + str(self.response_counter))
        print(SEP)

    def parse_pcap(self):
        """
        Function that's responsible of parsing the pcap , and creating a fixed pcap
        with just correct Roy's Protocol packets
        :return: Returns True if the parsing was operated successfully , otherwise returns False
        """
        ret_val = False
        try:
            pkt_list = rdpcap(PCAP_NAME)
            fixed_pcap_list = []
            for pkt in pkt_list:
                if Raw in pkt:
                    packet_payload = pkt[Raw].load
                    ret_val = self.check_pkt(packet_payload)
                    if ret_val:
                        fixed_pcap_list.append(pkt)
            self.new_pcap_descriptor(fixed_pcap_list)
            ret_val = True
        except FileNotFoundError:
            print(FILE_OPEN_ERROR)
        return ret_val

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
        conn.close()
        return ret_val
