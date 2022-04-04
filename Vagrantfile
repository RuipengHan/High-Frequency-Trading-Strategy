Vagrant.configure("2") do |config|
  
    config.vm.provider "virtualbox"
  
    config.vm.box = "IE498hftGroup1VM"
    config.vm.box_url = ENV["SS_BOX"]
    config.vm.box_download_checksum = "6900efe390b0c493a4f1ddd9ea9a1f28c357cc5b80324070d30ee06a8108db36"
    config.vm.box_download_checksum_type = "sha256"
    config.ssh.insert_key = false
  
    
    config.vm.define "stratdev" do |sd|
      sd.vm.hostname = "IE498hftGroup1VM"
      
      sd.vm.provider :virtualbox do |vb|
        vb.customize ["modifyvm", :id, "--memory", "1024"]
        vb.customize ["modifyvm", :id, "--cpus", "1"]
      end
    
      sd.vm.provision "file", source: ENV["SS_LICENSE"], destination: "/home/vagrant/Desktop/strategy_studio/backtesting/license.txt"
      sd.vm.provision "file", source: "requirements.txt", destination: "/home/vagrant/Desktop/requirements.txt"
      sd.vm.provision "shell", inline: "ls -lhs" # checks that vm is actually started
      sd.vm.provision "shell", inline: "/usr/local/bin/pip3.7 install -r /home/vagrant/Desktop/requirements.txt" # install additional python packages
    end
    
  end
  