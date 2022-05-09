'''
Alpaca data parser
'''

import os
from datetime import date, timedelta
import argparse
import alpaca_trade_api as tradeapi

parser = argparse.ArgumentParser(description='Alpaca data parser')
parser.add_argument('tickers', metavar='T', type=str, nargs='+',
                    help='tickers to obtain')
parser.add_argument('start', type=str,
                    help='start date of data to obtain in format YYYYMMDD')
parser.add_argument('end', type=str,
                    help='end date of data to obtain in format YYYYMMDD')
parser.add_argument('--output', dest="output", type=str, default="./",
                    help='directory to save output')
parser.add_argument('--mode', dest='mode', default="T",
                    help='Type of data to obtain. T for trade. Q for quote')

MKT_CENTER_CONVERT = {
    "A": "AMEX", 	        #   NYSE American (AMEX)
    "B": "BSE", 	        #   NASDAQ OMX BX; Used to be boston stock exchange.
    "C": "NSX",             #   National Stock Exchange; now NSYE national
    "D": "FINRA", 	        # 	FINRA ADF
    "E": None,              #   Market Independent
    "H": "MIAMI_OPTIONS", 	# 	MIAX
    "I": "ISE", 	        # 	International Securities Exchange
    "J": "EDGA", 	        # 	Cboe EDGA
    "K": "EDGX", 	        # 	Cboe EDGX
    "L": None,              #   Long Term Stock Exchange; Does not exists in 2010.
    "M": "CHICAGO", 	    # 	Chicago Stock Exchange; Now NYSE chicago
    "N": "NYSE", 	        # 	New York Stock Exchange
    "P": "Arca", 	        # 	NYSE Arca
    "Q": "NASDAQ_FUTURES",  #   NASDAQ OMX
    "S": "NASDAQ",          #   NASDAQ Small Cap; Should be XNCM but SS don't have it.
    "T": "NASDAQ",          #   NASDAQ Int; Should be XNIM but SS don't have it.
    "U": None,              #   Members Exchange; Does not exists in 2010.
    "V": "IEX", 	        # 	IEX
    "W": "CBOESTOCK", 	    # 	CBOE
    "X": "PHLX", 	        # 	NASDAQ OMX PSX
    "Y": "BYX", 	        # 	Cboe BYX
    "Z": "BATS",            #   Cboe BZX
}


class AlcapaParser:
    '''
    Alcapa data parser initializer
    Args:
        key_id: api key for alpaca. will look for $APCA_API_KEY_ID if not given
        secret: api key secret for alpaca, will look for $APCA_API_SECRET_KEY if not given
    '''
    def __init__(self, key_id = None, secret = None):
        self._api = tradeapi.REST(key_id = key_id,
                                secret_key = secret,
                                base_url = 'https://data.alpaca.markets/v2')


    def get_trade(self, tick:str, start_date:str, end_date:str, dest_dir = "./trades") -> None:
        '''
        A function that obtains trade data from alcapa API.
        Will download ALL trading data from start date to end date.
        The data will be parsed into SS format and save to dest_dir.
        One CSV will be created each date.
        WARNING:
            does not check if start time is before end time
        Args:
            tick: tick to download
            start_date: start date to obtain data in form YYYYMMDD
            end_date: end date to obtain data in form YYYYMMDD
            dest_dir: destination folder to store data
        '''
        if not os.path.isdir(dest_dir):
            raise ValueError(f"{dest_dir} does not exist")

        # calc time
        start = date(int(start_date[0:4]), int(start_date[4:6]), int(start_date[6:]))
        end = date(int(end_date[0:4]), int(end_date[4:6]), int(end_date[6:]))
        delta = end - start   # returns timedelta

        # for everyday in range
        for i in range(delta.days + 1):
            cur_day = start + timedelta(days=i)
            print(f"download day {cur_day}")

            month = cur_day.month if cur_day.month >= 10 else f"0{cur_day.month}"
            day = cur_day.day if cur_day.day >= 10 else f"0{cur_day.day}"
            output = os.path.join(dest_dir,
                    f"tick_{tick}_{cur_day.year}{month}{day}.txt")

            # Obtain raw data.
            raw_data_df = self._api.get_trades(tick,
                    f"{cur_day.isoformat()}",
                    f"{cur_day.isoformat()}").df
            if len(raw_data_df.index) == 0:
                print(f"No trade data on {cur_day}")
                continue
            raw_data_df.reset_index(inplace = True)

            # select necessary columns
            new_df = raw_data_df[['timestamp', 'timestamp', 'id', 'exchange', 'price', 'size']]
            print(f"got message of {raw_data_df.shape}, outputing to {output}")

            # insert T and convert exchange center
            new_df.insert(3, "tick_type", "T")
            new_df.replace({'exchange':MKT_CENTER_CONVERT}, inplace=True)
            new_df.dropna(inplace=True)
            print(f"tick_{tick}_{cur_day.year}{month}{day} trades csv outputted")
            new_df.to_csv(output, index=False)

    def get_quote(self, tick, start_date, end_date, dest_dir = "./quotes"):
        '''
        A function that obtains quotes data from alcapa API.
        Will download ALL quotes data from start date to end date.
        The data will be parsed into SS format and save to dest_dir.
        One CSV will be created each date.
        WARNING:
            does not check if start time is before end time
        Args:
            tick: tick to download
            start_date: start date to obtain data in form YYYYMMDD
            end_date: end date to obtain data in form YYYYMMDD
            dest_dir: destination folder to store data
        '''
        if not os.path.isdir(dest_dir):
            raise ValueError(f"{dest_dir} does not exist")
        # calc time
        start = date(int(start_date[0:4]), int(start_date[4:6]), int(start_date[6:]))
        end = date(int(end_date[0:4]), int(end_date[4:6]), int(end_date[6:]))
        delta = end - start   # returns timedelta

        # for everyday in range
        for i in range(delta.days + 1):
            cur_day = start + timedelta(days=i)
            print(f"download day {cur_day}")

            month = cur_day.month if cur_day.month >= 10 else f"0{cur_day.month}"
            day = cur_day.day if cur_day.day >= 10 else f"0{cur_day.day}"
            output = os.path.join(dest_dir,
                    f"tick_{tick}_{cur_day.year}{month}{day}.txt")
            # Obtain raw data.
            raw_data_df = self._api.get_quotes(tick,
                    f"{cur_day.isoformat()}",
                    f"{cur_day.isoformat()}").df
            if len(raw_data_df.index) == 0:
                print(f"No trade data on {cur_day}")
                continue
            raw_data_df.reset_index(inplace = True)

            # select necessary columns
            new_df = raw_data_df[['timestamp', 'timestamp',
            'ask_exchange', 'ask_price', 'ask_size',
            'bid_exchange', 'bid_price', 'bid_size', 'conditions']]
            new_df['conditions'] = new_df['conditions'].astype("string")
            print(f"got message of {raw_data_df.shape}, outputing to {output}")

            # insert T and convert exchange center
            new_df.insert(2, "tick_type", "Q")
            new_df.reset_index(inplace=True)
            new_df= new_df.rename(columns={"index":"seq_num"})
            new_df = new_df[['timestamp', 'timestamp', 'seq_num','tick_type',
                'ask_exchange','bid_exchange','bid_price', 'bid_size','ask_price', 'ask_size']]

            new_df.replace({'ask_exchange':MKT_CENTER_CONVERT}, inplace=True)
            new_df.replace({'bid_exchange':MKT_CENTER_CONVERT}, inplace=True)
            new_df.dropna(inplace=True)
            print(f"tick_{tick}_{cur_day.year}{month}{day} quotes csv outputted")
            new_df.to_csv(output, index=False)

if __name__ == "__main__":
    args = parser.parse_args()
    if args.start > args.end:
        raise ValueError("Start date must be before end date")

    parser = AlcapaParser()
    if args.mode == "T":
        for t in args.tickers:
            parser.get_trade(t, args.start, args.end, args.output)
    elif args.mode == "Q":
        for t in args.tickers:
            parser.get_quote(t, args.start, args.end, args.output)
    else:
        raise ValueError("invalid mode")
