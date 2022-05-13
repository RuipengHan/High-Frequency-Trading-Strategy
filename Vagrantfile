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

      # copy backtesting config files and a clean strategy db
      sd.vm.provision "file", source: "dependencies/cmd_config.txt", destination: "/home/vagrant/Desktop/strategy_studio/backtesting/utilities/cmd_config.txt"

      # copy backtesting script and strategy
      sd.vm.provision "file", source: "strategy/compile_and_backtest.sh", destination: "/home/vagrant/Desktop/compile_and_backtest.sh"
      sd.vm.provision "shell", inline: "mkdir -p /home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy"
      sd.vm.provision "shell", inline: "chmod -R 777 /home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy"
      sd.vm.provision "file", source: "strategy/SwingStrategy/SwingStrategy.h", destination: "/home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy/SwingStrategy.h"
      sd.vm.provision "file", source: "strategy/SwingStrategy/SwingStrategy.cpp", destination: "/home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy/SwingStrategy.cpp"
      sd.vm.provision "file", source: "strategy/SwingStrategy/Makefile", destination: "/home/vagrant/Desktop/strategy_studio/localdev/RCM/StrategyStudio/examples/strategies/SwingStrategy/Makefile"

      # copy python dependencies
      sd.vm.provision "file", source: "requirements.txt", destination: "/home/vagrant/Desktop/requirements.txt"
      sd.vm.provision "shell", inline: "ls -lhs" # checks that vm is actually started
      
      # run jobs
      sd.vm.provision "shell", inline: "/usr/local/bin/pip3.7 install -r /home/vagrant/Desktop/requirements.txt" # install additional python packages
      sd.vm.provision "shell", inline: "bash /home/vagrant/Desktop/compile_and_backtest.sh -s '2019-10-30' -e '2019-10-30' -i 'SwingStrategy' -t 'SPY'"
    end
    
  end
  