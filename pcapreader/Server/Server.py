import socket
from scapy.all import *
import matplotlib.pyplot as plotter
import binascii

#MAGIC STRINGS
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

    def create_graph(self):
        """
        Function that prints a graph output of the packet received of roy's protocol(divided to request/response)
        :return: None - object output function
        """
        pie_labels = 'Request', 'Response'
        packets = self.request_counter + self.response_counter
        population_share = [(self.request_counter / packets) * 100, (self.response_counter / packets) * 100]
        my_colors = ['lightgreen', 'lightblue']
        figure_object, axes_object = plotter.subplots()
        axes_object.set_title('Packets Received')
        # Draw the pie chart
        axes_object.pie(population_share,
                       # The fields of the graph
                       labels=pie_labels,
                       # two points after the decimal
                       autopct='%1.2f',
                       startangle=90,
                       # colors for the graph
                       colors=my_colors
                       )
        # Aspect ratio - equal means pie is a circle
        axes_object.axis('equal')
        plotter.show()

    def print_dict(self):
        """
        This function purpose is to print out the dictionary that descirbes the lua file
        that the server got.
        :return: None - print function
        """
        for key, value in self.protocol_fields.items():
            print(key, ' : ', value)

    def read_lua(self):
        """
        This function purpose is to parse the lua file into the dictionary
        and to update the values of the number of hex letters to check in the dictionary
        :return: returns True , if the file was opened correctly and updated it the dictionary.
        Returns False otherwise.
        """
        ret_val = False
        try:
            lua_f = open(LUA_NAME, "rb")
            lines = lua_f.readlines()
            c_line = 0
            for line in lines:
                line = line.decode('utf-8')
                # needs to add all the other ifs for the types
                if line.find("uint8") > 0:
                    self.protocol_fields[c_line] = 2
                elif line.find("uint16") > 0:
                    self.protocol_fields[c_line] = 4
                c_line += 1
            self.print_dict()
            lua_f.close()
            ret_val = True
        except FileNotFoundError:
            print(FILE_OPEN_ERROR)
        return ret_val

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

    def check_pkt(self, pkt):
        """
        This function purpose is to check whether the packet is in the standards that i defined
        to it or no
        :param pkt: Raw TCP payload of the packet
        :return: returns True if the packet is in the standards i chose , will return False
        if there was a failed type , flag , seq or pdu.
        """
        pkt = binascii.hexlify(pkt)
        offset = 0
        p_type = pkt[offset:offset + self.protocol_fields[0]]
        print(SEP)
        if p_type.find(b'01') >= 0:
            print("Checking request..")
            ret_val = self.check_request(pkt)
        elif p_type.find(b'02') >= 0:
            print("Checking response..")
            ret_val = self.check_response(pkt)
        else:
            print(TYPE_ERROR)
            ret_val = False
        return ret_val

    def check_request(self, hex_pkt):
        """
        This function check if the pkt of request type is correct
        :param hex_pkt: the TCP payload as a hexadecimal string
        :return:Returns True whether the REQUEST type packet is correct
        and False if there was a fail at the flag,seq,pdu
        """
        ret_val = True
        offset = 0
        offset += self.protocol_fields[0]
        flag = hex_pkt[offset:offset + self.protocol_fields[1]]
        if flag.find(b'01') < 0 and flag.find(b'02') < 0:
            print(FLAG_ERROR)
            ret_val = False
        offset += self.protocol_fields[1]

        seq = hex_pkt[offset:offset + self.protocol_fields[2]]
        if seq.find(b'03') < 0:
            ret_val = False
            print(SEQ_ERROR)
        offset += self.protocol_fields[2]

        pdu = hex_pkt[offset:offset + self.protocol_fields[3]]
        offset += self.protocol_fields[3]
        if pdu.find(b'0101') < 0:
            print(PDU_ERROR)
            ret_val = False
        if ret_val:
            self.request_counter += 1
            print(ret_val)
        print(SEP)
        return ret_val

    def check_response(self, hex_pkt):
        """
        This function check if the pkt of request type is correct
        :param hex_pkt: the TCP payload as a hexadecimal string
        :return:Returns True whether the RESPONSE type packet is correct
        and False if there was a fail at the flag,seq,pdu
        """
        ret_val = True
        offset = 0
        offset += self.protocol_fields[0]
        flag = hex_pkt[offset:offset + self.protocol_fields[1]]
        if flag.find(b'01') < 0 and flag.find(b'02') < 0:
            print(FLAG_ERROR)
            ret_val = False
        offset += self.protocol_fields[1]

        seq = hex_pkt[offset:offset + self.protocol_fields[2]]
        if seq.find(b'03') < 0:
            ret_val = False
            print(SEQ_ERROR)
        offset += self.protocol_fields[2]

        pdu = hex_pkt[offset:offset + self.protocol_fields[4]]
        offset += self.protocol_fields[4]
        if pdu.find(b'0202') < 0:
            print(PDU_ERROR)
            ret_val = False
        if ret_val:
            self.response_counter += 1
            print(ret_val)
        print(SEP)
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
        ret_val = self.read_lua()
        if ret_val:
            ret_val = self.parse_pcap()
        conn.close()
        return ret_val



