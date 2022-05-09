'''
Analysis runner
'''

import os

from compare_strategy import CompareStrategy
from strategy_analysis import StrategyAnalysis

# BACK_Swing10_2022-05-09_172753_start_06-02-2021_end_06-04-2021_
# BACK_Swing11_2022-05-09_180350_start_06-10-2019_end_06-14-2019_order

my_strategies = CompareStrategy()

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
    strategy_file = input(
            "Please enter the Strategy Studio Export file path\t" +
            "(prefix of the files until fill, order and pnl.csv) \n"
        )
    result_files = []

    invalid_file = False
    # BACK_Swing11_2022-05-09_172753_start_06-02-2021_end_06-04-2021_
    # BACK_Swing11_2022-05-09_180350_start_06-10-2019_end_06-14-2019_
    for result_type in ["fill", "order", "pnl"]:
        file_path = strategy_file + result_type + ".csv"
        if not os.path.exists(file_path):
            if not os.path.exists("data/" + file_path):
                print(
                    "Invalid file, please make sure you have " +
                    file_path +
                    " in your current directory."
                )
                invalid_file = True
                break
            file_path = "data/" + file_path

        result_files.append(
            file_path
        )
    if invalid_file is True:
        print("Invalid file, please re-enter!\n")
        return None

    return result_files

def process_interactive():
    '''
    Interative mode for users to add strategy
    '''
    # while True:
    #     start_time, end_time = check_valid_time()
    #     if start_time is None or end_time is None:
    #         continue
    #     break

    print("Parsing Strategy studio export files")
    print("="*30)
    while True:
        result_files = parse_files()
        if result_files is None:
            continue

        strategy = StrategyAnalysis(
                    fill_file=result_files[0],
                    order_file=result_files[1],
                    pnl_file=result_files[2],
                    initial_value=10000000)
        my_strategies.add_strategy(strategy)
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
