import plotly.graph_objects as go
import plotly.express as px
import numpy as np
import pandas as pd

'''
Python Class that compares the two strategies
'''
class CompareStrategy():
    def __init__(self):
        self.strategy_dict = {}

    # Add a strategy to the dictionary
    def addStrategy(self, strategy):
        if strategy in self.strategy_dict.keys():
            print("Warning: Strategy already exist, overwriting")
        self.strategy_dict[strategy.name] = strategy

    # Return the user specified strategy
    def getStrategy(self, name=None):
        if name == None or not name in self.strategy_dict.keys():
            return self.strategy_dict
        return self.strategy_dict
    
    # get Strategy in Dataframe Formate
    def getStrategiesDF(self):
        dict_key = list(self.strategy_dict.keys())
        strategy_columns = []
        for key in dict_key:
            strategy_pnl = self.strategy_dict[key].getData("pnl")['Cumulative PnL'].to_list()
            strategy_columns.append([key] + strategy_pnl)

        strategy_columns = np.array(strategy_columns).T
        strategy_df = pd.DataFrame(strategy_columns[1:], columns=strategy_columns[0])
        strategy_df.index = self.strategy_dict[dict_key[0]].getData("pnl")['Time'].to_list()
        strategy_df.index.name = "Date"
        strategy_df.columns.name = "Company"
        return strategy_df
    
    # Visualize the strategies
    def visualizeStrategies(self):
        strategies = self.getStrategiesDF()
        fig = px.area(strategies, facet_col="Company", facet_col_wrap=2)
        fig.show()

    # Get the Measurements for each strategy as a Dataframe
    def getMeasurements(self):
        dict_key = list(self.strategy_dict.keys())
        strategy_columns = []
        for key in dict_key:
            strategy_measurements = self.strategy_dict[key].measureStrategy()
            strategy_columns.append([key] + strategy_measurements)
        measurement_columns = ["Strategy Names", "Initial Value", "Final Value", "Begin Time", "End Time", "Net PnL", "Max PnL", "Min PnL", "Cumulative Returns", "Sharpe Ratio", "Max DrownDown"]
        strategy_df = pd.DataFrame(strategy_columns, columns= measurement_columns)
        return strategy_df
    
    # Output the Table with measurement for each strategy
    def measurementTable(self):
        measurement_df = self.getMeasurements()
        row_names = measurement_df.columns.to_numpy()[1:].reshape((10, 1)).T
        row_values = measurement_df.to_numpy()[:, 1:]
        rows = np.concatenate((row_names, row_values), axis=0)
        fig = go.Figure(data=[go.Table(header=dict(values=["Types"] + measurement_df["Strategy Names"].to_list()),
            cells=dict(values=rows))
        ])
        fig.show()


