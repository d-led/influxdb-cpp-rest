Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/trusty64"
  config.vm.provision "shell", privileged: true, inline: <<-SHELL
    apt-get update
    apt-get -y install git build-essential
    echo cd /vagrant >> `pwd`/.profile
  SHELL

  config.vm.provision "shell", privileged: false, inline: <<-SHELL
    ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Linuxbrew/install/master/install)"
    PATH="/home/linuxbrew/.linuxbrew/bin:$PATH"
    echo 'export PATH="/home/linuxbrew/.linuxbrew/bin:$PATH"' >>~/.bash_profile
    brew install gcc cmake cpprestsdk influxdb
  SHELL

  config.vm.provider "virtualbox" do |v|
    v.memory = 2048
    v.cpus = 2
  end
end
