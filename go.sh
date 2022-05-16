if [ -e dependencies/license.txt ]; then
	echo "Found license."
else
	echo "Copy your Strategy Studio licence to dependencies/license.txt before running."
	exit
fi

if [ -e dependencies/IE498hftGroup1VM.box ]; then
	echo "Found vagrant box"
else
	echo "You need to download IE498hftGroup1VM.box and place it at dependencies/IE498hftGroup1VM.box."
	exit
fi

if [ -e dependencies/cmd_config.txt ]; then
	echo "Found cmd config"
else
	echo "You need to place Strategy Studio cmd_config file at dependencies/cmd_config.txt."
	exit
fi

echo "All dependencies satisfied, start to vagrant"
vagrant up
echo "Finish vagrant provisioning, start download SPY data on 20220405"
sleep 5
vagrant ssh -c 'bash /home/vagrant/Desktop/download_from_alpaca.sh'
echo "Finish downloading data, start backtesting SwingStrategy"
sleep 5
vagrant ssh -c "bash /home/vagrant/Desktop/compile_and_backtest.sh -s '2022-04-05' -e '2022-04-05' -i 'SwingStrategy' -t 'SPY'"
echo "Done backtesting, start running analysis"
vagrant ssh -c "python3.7 /home/vagrant/Desktop/main.py"
echo "Done generating visualizations, files in VM, under /home/vagrant/Desktop/figs"
