# IE 498 Group 1 Project Proposal

Author: Ruipeng Han, Yihong Jian, Tommy Kimura, Zihan Zhou

## Introduction:

We will implement HFT strategies that trade on real-world market data using proprietary software Strategy Studio (SS in the following) for the final project. This goal has several components: data, strategy, analysis, and automation. The first component is data. Since SS does not have built-in data, we need to harvest our own for backtesting. Therefore, we will be downloading historic market data from IEX, Nasdaq database, and alpaca. The second component is the strategy itself--implement trading strategies provided by the professor and run them in SS. The next component will be interpreting the output of SS. SS will generate complex CSV files containing trade histories and earnings. Since the CSVs are hard to read, we will interpret them and analyze the strategies' performance, generating comparisons and visualizations. The final component is automation. Since running SS backtesting requires users to type many lines of commands, we feel the need to write a script to replace the manual work. Ideally, we should be able to run the backtesting in one click.

## Components & Technologies:
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

### Python Wrapper (optional)

- We could extend the `stragety.h` interface using python
  - SWIG
- If there’s still time left when we finish

## Evaluation & Goal:

- Clarity & understandability
  - Document process and week meetings
  - Codes are readable and follow google code styles standard
  - Proper use of pull requests
- Data
  - Can generate usable data
  - Runtime is reasonable
- Strategy
  - Strategy can run
  - Strategy outperforms holding the ticker
  - Strategy outperforms ExampleStrategy
- Interpretation & Visualization
  - Produce human readable report from raw csv
  - Graphs are clear and understandable
- Automation
  - “Oneclick” environmental setup
  - “Oneclick” backtesting
  - Reasonable tests and CI/CD
- Reach goal
  - Python wrapper
  - Streaming data & testing

## Timeline/Milestone & Work Distribution:

We established a milestone for each week until the end of the semester. Each milestone contains some tasks that should be completed on that date before class.

### Milestone 1: Processing data and backtesting result

**Timeline: March 26th - April 15th**

#### Parse IEX/Nasdaq data (Ruipeng)

- Understand Prof’s script and make sure it can run
- Retrieve the Nasdaq data
- Understand the raw data format
- Understand the format requirements from Strategy Studio
- Writing the actual parser that cleans the data and converts the raw data into the targeted data format

#### Parse Alpaca Data (Zihan)

- Figure out how having NBBO only would affect SS
- Retrieve the data
- Understand the raw data format
- Understand the format requirements from Strategy Studio
- Writing the actual parser that cleans the data and converts the raw data into the targeted data format

#### Analyze & Visualize Backtesting result (Tommy)

- (set up performance standards, ie what’s good/poor)
- Implement a backtesting analysis programFinalize what types of visualizations we need
- Implement the visualization code that output meaningful statistical figures 

#### DevOps (Yihong)

- Automate VM/environment setup
- Setup CI/CD procedures
- Automatically launch
- Some forms of unit-testing that assures each component works

### Milestone 2: Implement strategies and improve them

**Timeline: April 15th - April 29th**

We have yet assigned task for milestone 2, but we will redistribute work depending on number of strategies we need to implement and the potential problems we would face in Milestone 1.

#### Strategy Implementation

- Understand the interface
- Implement the strategies based on Professor’s feedback

#### DevOps

- Complete the automation process
- Implement the tests for our strategies

#### Analysis & Conclusion

- Thoroughly test the implementation of our strategies
- Debug & Improvement
- Draw analysis on these strategies with visuals and conclusions

### Milestone 3: Wrap up & Extend goal

**Timeline: April 29th - May 6th**

### Python Wrapper (Reach goal)

### Alpaca Streaming (Reach goal)

- Stream live data from Alcapa for lvie market data testing

### Overview

- Documentation
- Verbal