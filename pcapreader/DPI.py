import pandas as pd
import numpy as np
import sys

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

    def __init__(self):
        self.df_smalltrain = pd.read_csv("CsvFiles/AlgoTest.csv")
        self.df_stest = pd.read_csv("CsvFiles/Attributes.csv") #the received to check file
        self.df_results = pd.read_csv("CsvFiles/Results.csv")
        self.min_max = []

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
        check_pd_fix = check_pd.drop(["num_class", "p_type_y", "ID"], axis=1)
        check_pd_fix.rename(columns={'p_type_x': 'p_type'}, inplace=True)
        check_pd_fix.to_csv("CsvFiles/Check.csv")
        print(check_pd_fix)
        columns = check_pd_fix.columns.tolist()
        sys.stdout = open("Output/log.txt", "w")
        for index, row in check_pd_fix.iterrows():
            lst = row.tolist()
            print("-------------------------------------------")
            print(lst)
            for i in range(len(self.min_max) - 1):  # -1 so it won't run on the class
                if lst[i] < self.min_max[i][0]:
                    print(f"Lower than {self.min_max[i][0]} in {columns[i]}")
                elif lst[i] > self.min_max[i][1]:
                    print(f"Higher than {self.min_max[i][1]} in {columns[i]}")
        sys.stdout.close()

    def initialize(self):
        self.stage2()
        self.stage3()
        self.stage4()








