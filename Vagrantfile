Vagrant.configure("2") do |config|
  
    config.vm.provider "virtualbox"
  
    config.vm.box = "IE498hftGroup1VM"
    config.vm.box_url = "dependencies/IE498hftGroup1VM.box"
    config.vm.box_download_checksum = "6900efe390b0c493a4f1ddd9ea9a1f28c357cc5b80324070d30ee06a8108db36"
    config.vm.box_download_checksum_type = "sha256"
    config.ssh.insert_key = false
  
    
    config.vm.define "stratdev" do |sd|
      sd.vm.hostname = "IE498hftGroup1VM"
      
      sd.vm.provider :virtualbox do |vb|
        vb.customize ["modifyvm", :id, "--memory", "2048"]
        vb.customize ["modifyvm", :id, "--cpus", "2"]
      end
    
      # license
      sd.vm.provision "file", source: "dependencies/license.txt", destination: "/home/vagrant/Desktop/strategy_studio/backtesting/license.txt"

      # copy backtesting config files
      sd.vm.provision "file", source: "dependencies/cmd_config.txt", destination: "/home/vagrant/Desktop/strategy_studio/backtesting/utilities/cmd_config.txt"

      # copy backtesting script and strategy
      sd.vm.provision "file", source: "strategy/compile_and_backtest.sh", destination: "/home/vagrant/Desktop/compile_and_backtest.sh"
      sd.vm.provision "shell", inline: "mkdir -p /home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy"
      sd.vm.provision "shell", inline: "chmod -R 777 /home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy"
      sd.vm.provision "file", source: "strategy/SwingStrategy/SwingStrategy.h", destination: "/home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy/SwingStrategy.h"
      sd.vm.provision "file", source: "strategy/SwingStrategy/SwingStrategy.cpp", destination: "/home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy/SwingStrategy.cpp"
      sd.vm.provision "file", source: "strategy/SwingStrategy/Makefile", destination: "/home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy/Makefile"

      # copy alpaca data parsing script
      sd.vm.provision "file", source: "parser/alpaca_parser/alpaca_parser.py", destination: "/home/vagrant/Desktop/alpaca_parser.py"
      sd.vm.provision "file", source: "parser/download_from_alpaca.sh", destination: "/home/vagrant/Desktop/download_from_alpaca.sh"

      # copy visualization scripts
      sd.vm.provision "file", source: "analysis/compare_strategy.py", destination: "/home/vagrant/Desktop/compare_strategy.py"
      sd.vm.provision "file", source: "analysis/strategy_analysis.py", destination: "/home/vagrant/Desktop/strategy_analysis.py"
      sd.vm.provision "file", source: "analysis/main.py", destination: "/home/vagrant/Desktop/main.py"

      # install python dependencies
      sd.vm.provision "file", source: "requirements.txt", destination: "/home/vagrant/Desktop/requirements.txt"
      sd.vm.provision "shell", inline: "ls -lhs" # checks that vm is actually started
      sd.vm.provision "shell", inline: "/usr/local/bin/pip3.7 install -r /home/vagrant/Desktop/requirements.txt" # install additional python packages
    end
    
  end
  