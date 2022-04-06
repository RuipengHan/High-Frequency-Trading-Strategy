from cProfile import label
import os
import string
from matplotlib import dates
import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
import plotly.graph_objects as go
import plotly.express as px

'''
Python Class that takes in the fill, order, and pnl files generated by Strategy Studio Backtesting software and then have different visualiaztion functions
TODO Finish Fill and Order information
'''
class StrategyAnalysis:
    # @param fill_file: fill file generated by Strategy Studio Backtesting
    # @param order_file: Order file generated by Strategy Studio Backtesting
    # @param pnl_file: PnL file generated by Strategy Studio Backtesting
    def __init__(self, fill_file, order_file, pnl_file):
        # Filepath check
        self.checkPath(fill_file)
        self.checkPath(order_file)
        self.checkPath(pnl_file)
        # read these files into Dataframe
        try:
            self.fill = pd.read_csv(fill_file)
        except:
            raise Exception('Invalid fill file %s' %(fill_file))
        try:
            self.order = pd.read_csv(order_file)
        except:
            raise Exception('Invalid order file %s' %(order_file))
        try:
            self.pnl = pd.read_csv(pnl_file)
        except:
            raise Exception('Invalid pnl file %s' %(pnl_file))
            
    # Visualizat different metric standard by passing in the metrics
    # TODO Setup visualization for different evaluation standards
    # @param metric: types of metric the user would like to visualize, could either to be [], or simply one 
    '''
        metric: 'price'
    '''
    def visualization():
        print("Visualization")
    
    # Visualize PnL CSV file 
    # TODO Update the interpretation and fix the x label
    def visualizePNL(self):
        pnl_ = self.pnl[['Time', 'Cumulative PnL']]
        time_pnl = pnl_.to_numpy()
        N = time_pnl.shape[0]
        date_data = time_pnl[:, 0]
        cumulative_pnl = time_pnl[:, 1]
        time_point = np.arange(1, N + 1)

        time_series_fig = px.bar(pnl_, x=pnl_['Time'], y="Cumulative PnL")
        time_series_fig.show()

        date_label = [date_data[0].split(' ')[0]]
        pnl_by_date = []
        temp = [cumulative_pnl[0]]
        for i in range(1, N):
            date = date_data[i].split(' ')[0]
            if date == date_label[-1]:
                temp.append(float(cumulative_pnl[i]))
            else:
                date_label.append(date)
                pnl_by_date.append(np.array(temp))
                temp = [cumulative_pnl[i]]
        pnl_by_date.append(temp)
        pnl_by_date = pnl_by_date

        pnl_open = [arr[0] for arr in pnl_by_date]
        pnl_last = [arr[-1] for arr in pnl_by_date]
        pnl_high = [max(arr) for arr in pnl_by_date]
        pnl_low = [min(arr) for arr in pnl_by_date]

        bar_fig = go.Figure(data=[
            go.Candlestick(
                x=date_label,
                open=pnl_open,
                high=pnl_high,
                low=pnl_low,
                close=pnl_last)
        ])

        bar_fig.show()


    # Validate the file path
    # @param file_path: path of the file to check the existence
    def checkPath(self, file_path):
        if not os.path.exists(file_path):
            print("Invalid file path!")
            exit()
            
    # Get relevant data by type
    # @param types: types of the data to get, could be None for all, could be either fill, order, or pnl. Otherwise, it will return all
    def getData(self, types = None):
        if types == None or type(types) != string:
            return [self.fill, self.order, self.pnl]
        types = types.lower()
        if types == "fill":
            return self.fill
        elif types == "order":
            return self.order
        elif types == "pnl":
            return self.pnl
        return [self.fill, self.order, self.pnl]
