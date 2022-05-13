#!/bin/bash

# set APCA api_key and id
export APCA_API_KEY_ID="YOUR_ID"
export APCA_API_SECRET_KEY="YOUR_SECRET_KEY"

python3.7 /home/vagrant/Desktop/alpaca_parser.py SPY 20220405 20220405 --mode=T --output='/home/vagrant/Desktop/strategy_studio/backtesting/text_tick_data'