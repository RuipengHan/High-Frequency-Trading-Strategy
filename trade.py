import alpaca_trade_api as tradeapi
from alpaca_trade_api import TimeFrame
#define logger
import logging
logging.basicConfig(format='%(asctime)s %(message)s', level=logging.INFO)

api_key = 'PKKDR1JXESL35LP48EQF'
api_secret = 'tH7rGWihZcS2NcMo1mC0mX9ZebTPHINYxAv9hkZv'
base_url = 'https://paper-api.alpaca.markets'

# instantiate REST API
api = tradeapi.REST(api_key, api_secret, base_url, api_version='v2')


# obtain account information
account = api.get_account()
##print(api.request)
print(account.status)

def process_bar(bar):
    # process bar
    print(bar)

bar_iter = api.get_bars_iter("AAPL", TimeFrame.Hour, "2021-06-08", "2021-06-08", adjustment='raw')
for bar in bar_iter:
    process_bar(bar)
##to be matched with docs


#The Quotes API provides NBBO quotes for a single given ticker symbol at a specified date.
def process_quote(quote):
    # process quote
    print(quote)

quote_iter = api.get_quotes_iter("AAPL", "2021-06-08", "2021-06-08", limit=10)
for quote in quote_iter:
    process_quote(quote)
 ##printed in dataframe
print(api.get_quotes("AAPL", "2021-06-08", "2021-06-08", limit=10).df)

def process_trade(trade):
    # process trade
    print(trade)

trades_iter = api.get_trades_iter("AAPL", "2021-06-08", "2021-06-08", limit=10)
for trade in trades_iter:
    process_trade(trade)
  ## printed in dataframe
print(api.get_trades("AAPL", "2021-06-08", "2021-06-08", limit=10).df)  
