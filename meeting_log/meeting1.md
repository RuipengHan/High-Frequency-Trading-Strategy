## 1. Captain

me (Yihong)

## 2. Git access screenshot

 ![image-20220224212850420](commit_hist.png)

## 3. Project Ideas

1. Paper money trading algo

   Predict price on next tic based on past data (OHLC + some other indicators (MAs, MACD, Vol etc.)). We gonna approach this by first predicting the price (or movement) using time series analysis skills, and then develop algos to trade tickers.

   - Skillsets
     - Stochastic process?
     - Python
     - Some deep learning
     - Data processing
     - Crawler?
   - Testing
     - Back test on past market data
   - Some additional ideas
     - UI to adjust params?
     - Put it on live data
     - Automatic trading bot (Put reinforcement learning in it)
   - Problems
     - Where should we gather data?
     - What time scale should our algo runs on? (If using DL, the inference delay is significant)

2. Correlation between NASDAQ && IEX?

   Performance Analysis of IEX and / or NASDAQ. This idea comes from the PDF. We will differnet aspects of these two sets of data and generate visualizations of their performance.
   - Skillsets
     - Python
       - DS libs, Numpy, matplotlib, pandas, etc.
      - Statics/data analysis
    - Testing
      - Probably less relavant in this project
    - Difficulties
      - How do we know our result is actually correct?
      - What is the defintion of Performance?
        - i.e., what attirbute is most significant?

3. Implement TCP engine from scratch

    Implement a TCP connector using C and basic network library by using sockets and basic connections (maybe UDP?).
    - Skillsets
      - Network basics && how TCP works
        - These can derive from *Computer Networking, A Top Down Approach*.
      - C programming
        - We all know this from CS241/240
    - Testing
      - Running multiple VMs and setting network rules using ```tc```. 
    - Difficulties
      - Gauge performance?

## 4. Job for next week
  Everyone: investigate feasibility of our ideas.

## 5. Job of last week
  None.