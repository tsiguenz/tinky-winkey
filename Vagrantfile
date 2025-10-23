Vagrant.configure("2") do |config|

  config.vm.synced_folder '.', '/vagrant', disabled: true
  config.vm.define "win11", primary: true do |win11|
    win11.vm.box = "gusztavvargadr/windows-11"
    win11.vm.hostname = "win11"
  end

  config.vm.define "win10", autostart: false do |win10|
    win10.vm.box = "gusztavvargadr/windows-10"
    win10.vm.hostname = "win10"
  end

  config.vm.provider "virtualbox" do |vb|
    vb.gui = true
    vb.cpus = 4
    vb.memory = "8192"
    vb.customize ["modifyvm", :id, "--vram", "256"]
    vb.customize ["modifyvm", :id, "--accelerate-3d", "on"]
  end
end
