import pandas as pd
import numpy as np
import sys
from LuaHandler import *


def add_counter_label(frame):
    """
    Adds the ID column to the panda frame
    :param frame: a panda frame
    :return: a fixed panda frame with an additional id column
    """
    frame_len = len(frame)
    frame = frame.assign(ID=np.arange(0, frame_len))
    return frame


class ResultsDPI:
    """Class that represents the DPI checks"""

    def __init__(self, lua_handler: LuaHandler):
        self.lua_handler = lua_handler
        self.df_smalltrain = pd.read_csv("CsvFiles/AlgoTest.csv")
        self.df_stest = pd.read_csv("CsvFiles/Attributes.csv") #the received to check file
        self.df_results = pd.read_csv("CsvFiles/Results.csv")
        self.min_max = []
        self.interception_data = [] # for the values in the min-max who aren't actually in the learning stage

    def value_range_exists(self, series, min_value, max_value):
        """
        Function purpose is to find out which values of my basic min-max range
        are not actually in a corrected packet data
        :param series: a column from the dataframe
        :param min_value: min value of a specific column
        :param max_value: max value of a specific column
        :return: a list of all the values that were expected to be in a correct data but are not
        for example :
        unique [101,102,103,105]
        expected [101,102,103,104,105]
        not_in_range [104]
        """
        current_values = series.unique()
        expected_values = list(range(min_value, max_value + 1))
        # i want all the values of expected that are not in current
        not_in_range = np.setdiff1d(expected_values, current_values)
        print(f"unique : {current_values} \n expected : {expected_values} \n not_in_range : {not_in_range}")
        print("===========================================================================================")
        return not_in_range

    def prepare_unique_data(self, normal):
        """
        :param normal: normalized dataframe containing only packets from code 1 (means not anomalies)
        :return: None , updates self.interception_data and creates from it a list of lists for the
        values that were counted in the range but actually aren't part of it .
        """
        i = 0
        for (column_name, column_data) in normal.iteritems():
            min = self.min_max[i][0]
            max = self.min_max[i][1]
            print('Colunm Name : ', column_name)
            not_in_range_col = self.value_range_exists(normal[column_name], min, max)
            self.interception_data.append(not_in_range_col)
            i += 1

    def stage2(self):
        """ STAGE 2 - MIN <-> MAX [[min,max],[min,max]....,[min,max]] """
        grouped = self.df_smalltrain.groupby('num_class')
        normal = grouped.get_group(1)  # normal
        normal_max_list = list(normal.max())
        normal_min_list = list(normal[normal > 0].min())
        normal_min_list = list(map(int, normal_min_list))
        self.min_max = map(list, zip(normal_min_list, normal_max_list))
        print("=======================================")
        print("STAGE 2")
        self.min_max = list(self.min_max)
        print(self.min_max)
        self.prepare_unique_data(normal)

    def stage3(self):
        """ STAGE 3 - adding ID column to the test and result panda frames """
        self.df_stest = add_counter_label(self.df_stest)
        self.df_results = add_counter_label(self.df_results)
        print("=======================================")
        print("STAGE 3")
        print(f"{self.df_stest} \n {self.df_results} ")

    def stage4(self):
        """
        Stage 4 - DPI , compare between the two lists ID and check if each field is between
        the min and max of each attribute
        (taking anomaly from results (anomaly = 0))
        """
        print("=======================================")
        print("STAGE 4")
        # merges the prediction with their values
        check_pd = pd.merge(self.df_stest, self.df_results, on='ID')
        check_pd = check_pd[check_pd.num_class != 1]
        print(check_pd)

        pd_with_id = check_pd.drop(["num_class", "p_type_y"], axis=1)

        check_pd_fix = check_pd.drop(["num_class", "p_type_y", "ID"], axis=1)
        check_pd_fix.rename(columns={'p_type_x': 'p_type'}, inplace=True)
        print(check_pd_fix)
        columns = check_pd_fix.columns.tolist()
        sys.stdout = open("Output/log.txt", "w")
        incorrect_packets = open("Output/p_names.txt", "w")
        for index, row in pd_with_id.iterrows():
            lst = row.tolist()
            print("-------------------------------------------")
            print(f"(Packet Number  {row['ID'] + 1})")
            incorrect_packets.write("Packet " + str(row['ID'] + 1) + "\n")
            incorrect_packets.write("\n")
            print(lst)
            self.check_packet(lst, columns)
        sys.stdout.close()
        incorrect_packets.close()

    def find_msg_type_data(self, opcode):
        """
        Function that is responsible for creating a list of all the attributes to check by an opcode
        :param opcode: the decisive parameter for the protocol message types
        :return: returns a list of all the fields that are apart of the message type (including the generic msg headers)
        """
        msg_type_data = []
        for key, value_lst in (self.lua_handler.protocol_fields.items()):
            # checking the opcode
            if value_lst[2] == 0 or value_lst[2] == opcode:
                # adding the name of the field
                msg_type_data.append(value_lst[0])
            else:
                msg_type_data.append(" ")
        return msg_type_data

    def check_packet(self, lst, columns):
        """
        Function that is responsible for checking the packets using deep packet inspection.
        and printing to a file the output for a specific packet
        :param lst: the list that represents a packet data
        :param columns: my dataframe columns list
        :return: None , prints to a file
        """
        # if the decisive parameter is in the protocol types fields
        protocol_types_fields = self.lua_handler.protocol_types_fields
        if lst[0] != 0 and protocol_types_fields.get(lst[0]) is not None:
            # connecting between 0 opcode - msgheader and the msgtypedata
            msg_type_data = self.find_msg_type_data(lst[0])
            #print(f"message data {msg_type_data}")
            # goes over the protocol
            for key, value_lst in (self.lua_handler.protocol_fields.items()):
                # if the field is in my lst , check if the value fits the message type standards , else continue
                if value_lst[0] in msg_type_data:
                    if lst[key] in self.interception_data[key]:
                        print(f"Value in {columns[key]} is in the correct range of values but is not defined as one! ")
                    elif lst[key] < self.min_max[key][0]:
                        print(f" Lower than {self.min_max[key][0]} in {columns[key]} ")
                    elif lst[key] > self.min_max[key][1]:
                        print(f" Higher than {self.min_max[key][1]} in {columns[key]} ")
                else:
                    continue
        else:
            print("Packet is not built for the protocol !")


    def initialize(self):
        self.stage2()
        self.stage3()
        self.stage4()








