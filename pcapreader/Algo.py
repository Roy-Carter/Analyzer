from sklearn.multioutput import MultiOutputClassifier
from sklearn.model_selection import train_test_split, GridSearchCV
from sklearn import tree
from DPI import *


class MLAlgorithm:
    """Class that represents the ML algorithm"""
    def __init__(self):
        pass

    def start_algorithm(self):
        """Function to start running the train process and the output log afterwards"""
        test = pd.read_csv("CsvFiles/Attributes.csv")
        model = self.tree_train()
        self.test_classifier(model, test)
        dpi_output = ResultsDPI()
        dpi_output.initialize()

    def test_classifier(self, model, test):
        """
        This functions purpose is to classify the input csv file by the module and create
        an output csv file containing the algorithm results :
        duration , protocol type and a list of [num_proto ,num_class] for better clarification when reading.
        :param model: the trained module .
        :param test: the csv file we would like to check on the module
        :return: No returns , output withhin the function.
        """
        df_test = test
        test = df_test.drop(['p_type'], axis=1)
        t_pred = model.predict(test)
        class_p, protocol_p = self.edit_output(t_pred)
        check_results = pd.DataFrame({'num_class': class_p, 'p_type': protocol_p})
        print("===============================")
        check_results.to_csv("CsvFiles/Results.csv", index=False)

    @staticmethod
    def edit_output(t_pred):
        """
        receives the prediction output and transforms the prediction output to class and protocol lists
        :param t_pred: the prediction outputs
        :return: returns two lists of class prediction and protocol prediction
        """
        t_pred = list(t_pred)
        class_p = []
        protocol_p = []
        for arr_val in t_pred:
            class_p.append(arr_val[0])
            protocol_p.append((arr_val[1]))
        return class_p, protocol_p

    @staticmethod
    def grid_summary(final_data_test, x_test, x_train, y_test, y_train):
        """
        Summarizes the grid classification process .
        :param final_data_test: Best grid estimator tree , holds the best decision tree settings for most
        optimized results.
        :param x_test: holds the split data set for the testing part for the grid
        :param x_train: holds the split data set for the training part for the grid
        :param y_test: holds the label column to test for the grid
        :param y_train: holds the label column to train for the grid
        :return: No return .
        """
        print("===============================")
        print("Accuracy for the grid module :")
        print(f'Test:{final_data_test.score(x_test, y_test):.3f}')
        print(f'Train:{final_data_test.score(x_train, y_train):.3f}')

        best_feat = pd.DataFrame({'features': x_train.columns, 'importance': final_data_test.feature_importances_})
        print("===============================")
        print(best_feat.sort_values('importance', ascending=False))

    def tree_train(self):
        """
        This function trains a decision tree classifier model
        :return: a trained module
        """
        train = pd.read_csv("CsvFiles/AlgoTest.csv")
        df_working = train

        # y1 is my main target , multi label classification
        y1 = df_working[['num_class', 'p_type']]
        # this is a single target for plotting purpose
        y = df_working[['num_class']]
        """
        dropping my two targets
        """
        X = df_working.drop(['num_class', 'p_type'], axis=1)
        print("model description")
        print(X.describe())
        # for the multiout classification
        x_train1, x_test1, y_train1, y_test1 = train_test_split(X, y1, test_size=.2)
        # for the grid , finding the best tree option
        x_train, x_test, y_train, y_test = train_test_split(X, y, test_size=.2)
        dt_final = self.grid_search(x_train, x_test, y_train, y_test)
        model = self.multioutput_classifier(dt_final, x_train1, x_test1, y_train1, y_test1)
        return model

    def grid_search(self, x_test, x_train, y_test, y_train):
        split_range = list(range(2, 25))
        param_grid = {'criterion': ('gini', 'entropy'), 'max_depth': (np.arange(1, 10)),
                      'min_samples_leaf': split_range}
        dtc = tree.DecisionTreeClassifier()
        dt_grid = GridSearchCV(dtc, param_grid, cv=10, n_jobs=-1, verbose=2, scoring='accuracy')
        # fit the grid with data
        dt_grid.fit(x_train, y_train)
        # best estimator (the best decision tree settings basically)
        dt_final = dt_grid.best_estimator_
        dt_final.fit(x_train, y_train)
        # y_pred can be used for classification_report (in the print explanations.py)
        y_pred = dt_final.predict(x_test)
        self.grid_summary(dt_final, x_test, x_train, y_test, y_train)
        return dt_final

    def multioutput_classifier(self, dt_final, x_train1, x_test1, y_train1, y_test1):
        model = MultiOutputClassifier(dt_final)
        model.fit(x_train1, y_train1)  # training the model this could take a little time
        accuracy = model.score(x_test1, y_test1)  # comparing result with the test part set
        data = {'Accuracy': [accuracy], 'Algorithm': ['DecisionTreeClassifier']}
        algorithm_output = pd.DataFrame(data)
        print("===============================")
        print("Training Accuracy Using Multi label during train:")
        print(algorithm_output)
        return model

