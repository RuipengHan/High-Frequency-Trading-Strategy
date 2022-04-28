#!/bin/bash

helpFunction()
{
   echo ""
   echo "Usage: $0 -s startDate -e endData -i strategy instance name -t ticker"
   echo "Dates should be in format yyyy-mm-dd"
   echo "ticker should be in format SPY|AAPL|MSFT, if using only 1 ticker, ignore |"
   echo ""
   exit 1 # Exit script after printing help
}


# parse arguments
while getopts "s:e:i:t:" opt
do
   case "$opt" in
      s ) startDate="$OPTARG" ;;
      e ) endDate="$OPTARG" ;;
      i ) instanceName="$OPTARG" ;;
      t ) ticker="$OPTARG" ;;
      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
   esac
done

# check if arguments are valid
if [ -z "$startDate" ] || [ -z "$endDate" ] || [ -z "$instanceName" ] || [ -z "$ticker" ]
then
   echo "Invalid arguments";
   helpFunction
fi

# compile and copy dll
cd ~/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/$instanceName
make
cp $instanceName.so /home/vagrant/Desktop/strategy_studio/backtesting/strategies_dlls/

# create instance
cd ~/Desktop/strategy_studio/backtesting/
./StrategyServerBacktesting &
sleep 1
cd ./utilities/
./StrategyCommandLine cmd create_instance $instanceName $instanceName UIUC SIM-1001-101 dlariviere 1000000 -symbols $ticker
./StrategyCommandLine cmd strategy_instance_list

# run back test
./StrategyCommandLine cmd start_backtest $startDate $endDate $instanceName 0

# Get the line number which ends with finished. 
foundFinishedLogFile=$(grep -nr "finished.$" /home/vagrant/Desktop/strategy_studio/backtesting/logs/main_log.txt | gawk '{print $1}' FS=":"|tail -1)

# DEBUGGING OUTPUT
echo "Last line found:",$foundFinishedLogFile

# If the line ending with finished. is less than the previous length of the log file, then strategyBacktesting has not finished, 
# once its greater than the previous, it means it has finished.
while ((logFileNumLines > foundFinishedLogFile))
do
    foundFinishedLogFile=$(grep -nr "finished.$" /home/vagrant/Desktop/strategy_studio/backtesting/logs/main_log.txt | gawk '{print $1}' FS=":"|tail -1)

    #DEBUGGING OUTPUT
    echo "Waiting for strategy to finish" 
    sleep 1
done

echo "Sleeping for 10 seconds..."

sleep 10

echo "run_backtest.sh: Strategy Studio finished backtesting"



