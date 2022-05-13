'''
Analysis runner
'''

import os

from compare_strategy import CompareStrategy
from strategy_analysis import StrategyAnalysis

# BACK_Swing10_2022-05-09_172753_start_06-02-2021_end_06-04-2021_
# BACK_Swing11_2022-05-09_180350_start_06-10-2019_end_06-14-2019_order

my_strategies = CompareStrategy()
TICKER_DIRECTORY = "final_data/spy"

def parse_date(date_string, reverse=False):
    """
    Convert YYYYMMDD to YYYY-MM-DD

    Parameters
    ----------
        types : str
            date in YYYYMMDD string format
    """
    if len(date_string) != 8:
        return None

    if reverse:
        return f"{date_string[4:6]}-{date_string[6:]}-{date_string[0:4]}"
    return f"{date_string[0:4]}-{date_string[4:6]}-{date_string[6:]}"


def check_valid_time():
    '''
    Check validity of input time
    '''
    start_time = input("What is the desired start time? (YYYYMMDD)\n")
    end_time = input("What is the desired end time? (YYYYMMDD)\n")
    start_time_list = parse_date(start_time).split("-")
    end_time_list = parse_date(end_time).split("-")
    if start_time_list is None or end_time_list is None:
        print("Invalid start time and end time")
        return None, None
    for i in range(3):
        start_i = int(start_time_list[i])
        end_i = int(end_time_list[i])
        if start_i > end_i:
            print("Invalid start and end time..\n\n")
            return None, None
        if start_i < end_i:
            break
    return start_time, end_time

def parse_files():
    '''
    Parse input files
    '''
    strategy_name = input(
        "Please enter the name of your strategy\n"
        )
    strategy_name = f"_{strategy_name}_"
    result_files = []

    for file in os.listdir("sample_data"):
        if strategy_name in file:
            result_files.append("sample_data/" + file)

    if len(result_files) < 3:
        print("Invalid file, please re-enter!\n")
        return None

    if len(result_files) > 3:
        strategy_id = input(
            "Please enter the id of your strategy\n"
        )
        new_result_files = []
        for file in result_files:
            if strategy_id in file:
                new_result_files.append(file)
        result_files = new_result_files
        if len(result_files) < 3:
            print("Invalid file, please re-enter!\n")
            return None

    return_files = ["", "", ""]
    for target_file in result_files:
        if target_file[-8:] == "fill.csv":
            return_files[0] = target_file
        elif target_file[-9:] == "order.csv":
            return_files[1] = target_file
        elif target_file[-7:] == "pnl.csv":
            return_files[2] = target_file
    return return_files

def parse_ticks():
    '''
    Parse ticker files
    '''
    tick_name = input(
        "Please enter the name of the tickers\n"
        )
    tick_name = f"_{tick_name}_"
    tick_files = []
    for file in os.listdir(TICKER_DIRECTORY):
        if tick_name in file:
            tick_files.append(TICKER_DIRECTORY + "/" + file)
    return tick_files

def process_interactive():
    '''
    Interative mode for users to add strategy
    '''

    print("Parsing Strategy studio export files")
    print("="*30)
    while True:
        result_files = parse_files()
        if result_files is None:
            continue
        tick_files = parse_ticks()
        if tick_files is None:
            print("Invalid Tick files")
        strategy = StrategyAnalysis(
                    fill_file=result_files[0],
                    order_file=result_files[1],
                    pnl_file=result_files[2],
                    initial_value=10000000)
        strategy.read_ticks(tick_files)
        my_strategies.add_strategy(strategy)
        # strategy.measure_strategy()
        strategy.visualize_pnl()
        end = input(
            "Do you want to continue adding strategy? (yes / no)\n"
            ).strip().lower()
        if end in ('no', 'n'):
            break
    my_strategies.visualize_strategies()
    my_strategies.measurement_table()

if __name__ == "__main__":
    process_interactive()
