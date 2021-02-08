from scapy.all import *
import binascii
import pandas as pd

PCAP_NAME = "test.pcap"
LUA_NAME = "descriptor.lua"
FILE_OPEN_ERROR = "This file name does not exists"


def print_dict(val):
    """
    Prints the dictionary
    :param val: a dictionary to print
    :return: None.
    """
    for key, value in val.items():
        print(key, ' : ', value)


class LuaHandler:

    def __init__(self):
        """
        The constructor for the server class
        protocol_fields - will hold a dictionary to every lua line that is parsed
        protocol_types_fields - a dictionary where the key is the opcode and the values for each one are the attributes
        for that specific message type (0 - is for basic header though)
        opcode_list - list of all the opcodes of the messages types of the protocol (by the lua file)
        """
        self.protocol_fields = {}
        self.protocol_types_fields = {}
        self.opcode_list = []

    def create_msg_types(self):
        """
        This function creates a dictionary of list , which each key stands for a specific opcode and the list
        within it , holds all the parameters that are part of that specific message .
        :return: None.
        """
        self.protocol_types_fields = {self.opcode_list[k]: [] for k in range(len(self.opcode_list))}
        for i in range(len(self.protocol_fields)):
            opcode = self.protocol_fields[i][2]
            self.protocol_types_fields[opcode].append(self.protocol_fields[i][0])
        print_dict(self.protocol_types_fields)

    def read_lua(self):
        """
        Creates a dictionary of lists where the number of line is the key and the [<fieldname>,<size>] is the value
        :return: returns True if the file was opened correctly and the dictionary was created , False otherwise.
        """
        ret_val = False
        try:
            lua_f = open(LUA_NAME, "rb")
            c_line = 0
            for line in lua_f:
                line_values = line.split()
                dict_list = [line_values[0].decode()]
                size = line_values[1].decode()
                opcode = int(line_values[2].decode())
                if opcode not in self.opcode_list:
                    self.opcode_list.append(opcode)
                if size == 'uint8':
                    dict_list.append(2)
                elif size == 'uint16':
                    dict_list.append(4)
                elif size == 'uint24':
                    dict_list.append(6)
                elif size == 'uint32':
                    dict_list.append(8)
                else:
                    continue
                dict_list.append(opcode)
                self.protocol_fields[c_line] = dict_list
                c_line += 1
            print_dict(self.protocol_fields)
            print("======================================")
            self.create_msg_types()
            lua_f.close()
            ret_val = True
        except FileNotFoundError:
            print(FILE_OPEN_ERROR)
        return ret_val

    def convert_attributes_list(self):
        """
        Creates a list for the attributes name (the protocol fields)
        :return: the attributes list
        """
        attr_list = []
        for i in self.protocol_fields.values():
            attr_list.append(i[0])
        return attr_list

    def remove_csv_outcasts(self, df):
        for index, row in df.iterrows():
            print(row['c1'], row['c2'])

    @staticmethod
    def create_csv(packets_list, attr_list):
        """
        Creates and prints the csv file for the machine learning process
        :param packets_list: holds a list of lists of each packet values parsed.
        :param attr_list: a list of all the protocol attributes
        :return: None.
        """
        frame = pd.DataFrame(packets_list, columns=attr_list)
        frame.fillna(0, inplace=True, downcast='infer')
        frame.to_csv("CsvFiles/Attributes.csv", index=False)
        print("======================================")
        print(frame)

    def parse_pcap(self):
        """
        Parsing the pcap to a csv file.
        :return: return True if the file was parsed into csv , otherwise returns false.
        """
        ret_val = False
        attr_list = self.convert_attributes_list()
        protocols_list = []
        try:
            pkt_list = rdpcap(PCAP_NAME)
            for pkt in pkt_list:
                if Raw in pkt:
                    packet_payload = pkt[Raw].load
                    protocol_description = self.check_pkt(packet_payload)
                    protocols_list.append(list(protocol_description.values()))
            self.create_csv(protocols_list, attr_list)
            ret_val = True
        except FileNotFoundError:
            print(FILE_OPEN_ERROR)
        return ret_val

    def initialize_proto_dict(self):
        """
        This function creates an intizialize dictionary to use for the csv file where the key will be an attribute of the
        protocol and the initialized value to him will be zero.
        :return: returns a dictionary {<attr_list>..,<0>}
        """
        basic_list = []
        for i in range(len(self.protocol_types_fields)):
            # print(range(len(self.protocol_types_fields))) -- KeyError Exception 2/3 when changing opcode sometimes.
            basic_list += self.protocol_types_fields[i]

        zeros = ['0' for i in range(len(self.protocol_fields))]
        basic_dict = dict(zip(basic_list, zeros))
        return basic_dict

    def handle_msg_types(self, pkt, val_type, offset, lua_dict_index, packet_dict):
        """
        This function is responsible for taking care of specific message types (which aren't header values) and
        add them to a dictionary that represents for each packet {<field_name>:<value>,....} (updates the dictionary)
        :param pkt: the packet that I am currently checking
        :param val_type: the value of the first bytes of the protocol that needs to represent a message type
        :param offset: the current offset from the start of the packet
        :param lua_dict_index: the current index of the line from the lua dictionary that i'm checking.
        :param packet_dict: the dict to update each packet values to.
        :return:None
        """
        if val_type in self.protocol_types_fields.keys():
            for field_name in self.protocol_types_fields[val_type]:
                val = pkt[offset:offset + self.protocol_fields[lua_dict_index][1]]
                if val:
                    packet_dict[field_name] = val
                else: # needs this check to avoid having no values in extreme circumstances
                    packet_dict[field_name] = 0

                offset += self.protocol_fields[lua_dict_index][1]

    def check_pkt(self, pkt):
        """
        Checks each packet of the pcap file and creates a list of its data split to fields.
        :param pkt: a packet from the pcap file
        :return: returns a dictionary with the packet data for the protocol {<field_name>:<value>,....}
        """
        packet_dict = self.initialize_proto_dict()
        pkt = binascii.hexlify(pkt)
        pkt = pkt.decode()
        offset = 0
        flag = False
        val_type = int(pkt[offset:offset + self.protocol_fields[0][1]])
        for index in range(len(self.protocol_fields)):
            if not flag:
                op_code = self.protocol_fields[index][2]
                val = pkt[offset:offset + self.protocol_fields[index][1]]
                if op_code != 0:
                    self.handle_msg_types(pkt, val_type, offset, index, packet_dict)
                    flag = True
                else:
                    p_type = self.protocol_fields[index][0]
                    if val: # same check as in msg_type
                        packet_dict[p_type] = val
                    else:
                        packet_dict[p_type] = 0

                offset += self.protocol_fields[index][1]
        return packet_dict


