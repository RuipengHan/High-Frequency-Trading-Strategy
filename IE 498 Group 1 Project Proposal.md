# IE 498 Group 1 Project Proposal

Author: Ruipeng Han, Yihong Jian, Tommy Kimura, Zihan Zhou

## Introduction:

We will implement HFT strategies that trade on real-world market data using proprietary software Strategy Studio (SS in the following) for the final project. This goal has several components: data, strategy, analysis, and automation. The first component is data. Since SS does not have built-in data, we need to harvest our own for backtesting. Therefore, we will be downloading historic market data from IEX, Nasdaq database, and alpaca. The second component is the strategy itself--implement trading strategies provided by the professor and run them in SS. The next component will be interpreting the output of SS. SS will generate complex CSV files containing trade histories and earnings. Since the CSVs are hard to read, we will interpret them and analyze the strategies' performance, generating comparisons and visualizations. The final component is automation. Since running SS backtesting requires users to type many lines of commands, we feel the need to write a script to replace the manual work. Ideally, we should be able to run the backtesting in one click.

## Technologies:
### Data parsing

- Since SS will not recognize the raw IEX/Nasdaq/Alpaca data, we must implement a parser that converts raw data into a usable format. We decided to use Python to parse the data since it does not require high performance, and Python has more plug-ins for file interactions. For the alpaca data, we will obtain them using the official Python SDK, so we do not have to send web requests manually.
  - Python
  - Alpaca-trade-api

### Implement strategy

- Strategies will be implemented by extending StrategyStudio::Strategy interface using C++. In this way, we can compile binaries for SS to backtest.
  - C++
  - Strategy Studio

### Analysis

- Since analysis requires parsing output CSV files and generating plots, we determined that Python will best serve our interest due to the widely available data analysis packages. Some potential tools are:
  - Matplotlib
  - Seaborn
  - Pandas
  - Numpy

### Automation

- SS works by launching a separate terminal where users need to type in commands. Therefore, a logical approach to writing an automation script will be redirecting stdin and stdout. We feel that the two viable options are bash scripts using directional operators or Python with system calls.
  - Bash script
  - python script

## Timeline/Milestone:

We established a milestone for each week until the end of the semester. Each milestone contains some tasks that should be completed on that date before class.

#### March 31st

Parse IEX/Nasdaq data

#### April 7th

Parse Alpaca data

#### April 14th

Writing Strategy

#### April 21th 

Put data and strategy together/Debug/Analysis

#### April 28th

Analysis

#### May 5th

Automation

#### May 12th

Extra week for lags

## Job Distribution:
For job distribution, we split the work in each week’s regular meeting. However, each individual teammate will be mainly responsible for a certain task. Specifically, he/she should make sure the task is done for the week and is the go-to person for any issues on the task.

- Ruipeng Han
  - Strategy Implementation
- Yihong Jian
  - Automation/Testing
- Tommy Kimura
  - IEX/Nasdaq parsing & Analysis
- Zihan Zhou
  - Alpaca parsing & Analysis