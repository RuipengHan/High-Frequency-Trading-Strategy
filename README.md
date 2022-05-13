# group_01_project

This is the base git project for group 01 of IE498 HFT in Spring 2022.

## Teammates

Yihong Jian - yihongj2@illinois.edu (Team Leader)

Ruipeng Han - ruipeng2@illinois.edu

Zihan zhou - zihanz12@illinois.edu

Tomoyoshi (Tommy) Kimura - tkimura4@illinois.edu

## Environment

Standard development environment is provided using vagrant box.

### Software included
- Strategy Studio
- Python 3.7.11
  - Additional Packages should be included in ```requirements.txt```.

Please create a issue for additional softwares so that we can put it in vagrant provisioning.

### Usage:

1. Install Vagrant and virtualbox.
2. Download box using this [link](https://uofi.box.com/s/wlyq6b23k41dbw1bz7sfff631osp1049) that requires login from your university account.
3. Set directory of box file to environment variable ```SS_BOX```, for example:
    ```   bash
    export SS_BOX="/home/user/IE498hftGroup1.box
4. Set directory of strategy studio ```SS_LICENSE```, for example:
    ```   bash
    export SS_LICENSE="/home/user/license.txt
5. ``` bash
   vagrant up
6. Then use ```vagrant ssh``` to connect to VM

## Project structure
### Components
- **Market data parser**

    All the data parser (Nasdaq/IEX/alpaca) should be placed under ```parser``` folder. Implementation should follow the interface in ```parser_base.py```.
    - IEX
      - Description

        We directly imported Professor's IEX downloader/parser as a submodule of our project so we can conveniently retrive IEX data.
        This parser downloads and parses IEX DEEP and trade data into the "Depth Update By Price" and "Trade Message" format in Strategy Studio, respectively. 
        
        The original repository can be found <ins>[here](https://gitlab.engr.illinois.edu/shared_code/iexdownloaderparser)</ins>.
      
      - Directions
        - Installing Dependencies
          - pip3 install -r requirements.txt 
        - Usage
          1. Run <code>./download.sh</code> to download the source IEX deep data (.gz format).
            To retrieve data in a specific range of dates, open and edit the download.sh, only modifies the start-date and end-date arguments:

              <code> python3 src/download_iex_pcaps.py --start-date 2021-11-15 --end-date 2021-11-16 --download-dir data/iex_downloads </code>

          2. Check that the downloaded raw IEX DEEP dat files should be stored at iexdownloaderparsers/data/iex_downlaods/DEEP

          3. Run <code>./parse_all.sh</code> to parse IEX deep data. Result will be stored under iexdownloaderparsers/data/text_tick_data with the foramt tick_SYMBOL_YYYYMMDD.txt.gz.

              To specify the company symbols, edit the --symbols argument in parse_all.sh. The defualt is SPY only. You can add more companys:

              <code>gunzip -d -c $pcap | tcpdump -r - -w - -s 0 | $PYTHON_INTERP src/parse_iex_pcap.py /dev/stdin --symbols SPY,APPL,GOOG,QQQ --trade-date $pcap_date --output-deep-books-too</code>

          4. The parsed data is in .gz format. We want to extract it and save it to a txt file which can be feed into strategy studio, run the following command under iexdownloaderparsers/data/text_tick_data (*please change your symbol and dates accordingly*):

              <code>gunzip -d -c tick_SPY_20171218.txt.gz | awk -F',' '$4 == "P" {print $0}' > tick_SPY_20171218.txt</code>

              This command extracts the data and rows where the fourth column is "P", which corresponds to the format of "Depth Update By Price (OrderBook data)" in strategy studio. 

              If instead you want to retrive only the trade data, simply change "P" to "T" in the above command, which is following:

              <code>gunzip -d -c tick_SPY_20171218.txt.gz | awk -F',' '$4 == "T" {print $0}' > tick_SPY_20171218.txt</code>

          5. The tick_SPY_20171218.txt (*or your custom data file*) is ready to feed in SS.

    - NASDAQ

    - Alpaca


- **Strategy**

    All the strategies codes (.h, .cpp, makefile) should be included under ```strategy``` folder.

- **Analysis/visualization**

    Code should be included in ```visualization``` folder.
### Quality assurance
- **Code Sanity**
  - Python code will be checked by ```PyLint``` under PEP8 standard.
  - CPP checkstyle - TBD
- **Unit Testing**
  - TBD