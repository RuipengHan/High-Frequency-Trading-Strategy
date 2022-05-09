# IexDownloaderParser

This parser downloads and parses IEX DEEP data into the "Depth Update By Price" format in Strategy Studio.

# Dependencies (Please install all of them as well as dependencies from requirements.txt)

## Downloader (substitute with whatever version of python interpreter you are using)
1. pypy3.7 -m pip install requests
2. pypy3.7 -m pip install tqdm
3. yum install tcpdump

# Directions:
1. Run ./download.sh to download the source IEX deep data (.gz format). 

    To retrieve data in a specific range of dates, open and edit the download.sh, only modifies the start-date and end-date arguments:

      python3 src/download_iex_pcaps.py --start-date 2021-11-15 --end-date 2021-11-16 --download-dir data/iex_downloads

2. The downloaded raw IEX DEEP dat files should be stored at iexdownloaderparsers/data/iex_downlaods/DEEP

3. Run ./parse_all.sh to parse IEX deep data. Result will be stored under iexdownloaderparsers/data/text_tick_data with the foramt tick_SYMBOL_YYYYMMDD.txt.gz.

   To specify the company symbols, edit the --symbols argument in parse_all.sh. The defualt is SPY only. You can add more companys:

   gunzip -d -c $pcap | tcpdump -r - -w - -s 0 | $PYTHON_INTERP src/parse_iex_pcap.py /dev/stdin --symbols SPY,APPL,GOOG,QQQ --trade-date $pcap_date --output-deep-books-too

4. The parsed data is in .gz format. We want to extract it and save it to a txt file which can be feed into strategy studio, run the following command under iexdownloaderparsers/data/text_tick_data (please change your symbol and dates accordingly):

  gunzip -d -c tick_SPY_20171218.txt.gz | awk -F',' '$4 == "P" {print $0}' > tick_SPY_20171218.txt

  This command extracts the data and rows where the fourth column is "P", which corresponds to the format of "Depth Update By Price (OrderBook data)" in strategy studio.

5. The tick_SPY_20171218.txt (or your custom data file) is ready to feed in SS.