#!/usr/bin/python
import subprocess
import os
host=['lyoilcrpi03','lyoilcrpi04','lyoilcrpi05','lyoilcrpi06','lyoilcrpi07','lyoilcrpi08','lyoilcrpi09','lyoilcrpi10','lyoilcrpi11','lyoilcrpi12','lyoilcrpi14','lyoilcrpi15','lyoilcrpi19','lyoilcrpi17']
host=['lyoilcrpi03','lyoilcrpi04','lyoilcrpi05','lyoilcrpi06','lyoilcrpi07','lyoilcrpi08','lyoilcrpi09','lyoilcrpi10','lyoilcrpi11','lyoilcrpi12','lyoilcrpi14','lyoilcrpi24','lyoilcrpi19','lyoilcrpi17','lyoilcrpi15']
#host=['lyoilcrpi15']
#host=['lyoilcrpi18']
res=[]
for h in host:
  cmd="ssh pi@%s 'sudo /opt/dhcal/dimjc/etc/dimjcd stop'" %h
  os.system(cmd)
  cmd="ssh pi@%s 'sudo /opt/dhcal/dimjc/etc/dimjcd start'" %h
  os.system(cmd)
  """
  cmd ="rsync -axv /opt/dhcal/lib/ %s:/opt/dhcal/lib/" % h
  os.system(cmd)
  cmd ="rsync -axv /opt/dhcal/etc/ %s:/opt/dhcal/etc/" % h
  os.system(cmd)
  cmd ="rsync -axv /opt/dhcal/bin/ %s:/opt/dhcal/bin/" % h
  os.system(cmd)
  cmd ="rsync -axv /opt/dhcal/levbdim/ %s:/opt/dhcal/levbdim/" % h
  os.system(cmd)

  cmd="ssh pi@%s 'sudo /opt/dhcal/levbdim/bin/ljcd start'" %h
  os.system(cmd)


  cmd ="rsync -axv /opt/dhcal/dimjc/ %s:/opt/dhcal/dimjc/" % h
  os.system(cmd)
 
  #cmd="ssh pi@%s 'sudo apt-get -y install libjsoncpp-dev '" %h

  cmd="ssh pi@%s 'sudo apt-get update '" %h
  os.system(cmd)
  cmd="ssh pi@%s 'sudo apt-get -y install libmysqlclient-dev liblog4cxx10-dev  '" %h
  os.system(cmd)

  cmd="ssh pi@%s 'sudo killall -9 levbdim_zup'" %h
  os.system(cmd)
  cmd="ssh pi@%s 'sudo killall -9 levbdim_ccc'" %h
  os.system(cmd)
  cmd="ssh pi@%s 'sudo killall -9 levbdim_dif'" %h
  os.system(cmd)

  
  cmd="ssh pi@%s 'sudo rm -rf /var/log/pi'" %h
  os.system(cmd)
  cmd="ssh pi@%s 'sudo mkdir -p /var/log/pi'" %h
  os.system(cmd)
  cmd="ssh pi@%s 'sudo chmod 777 /var/log/pi'" %h
  os.system(cmd)
  cmd="ssh pi@%s 'sudo killall -9 dimrpc_dif'" %h
  os.system(cmd)
  cmd="ssh pi@%s 'sudo rm /tmp/dimjcPID*.log'" %h
  os.system(cmd)
  

  #cmd="ssh pi@%s 'sudo ln -sf /opt/dhcal/etc/Log4cxxConfig.xml /etc/Log4cxxConfig.xml'" %h
  #os.system(cmd)
 
  cmd="ssh pi@%s 'sudo /opt/dhcal/dimjc/etc/dimjcd stop'" %h
  os.system(cmd)
  cmd="ssh pi@%s 'sudo /opt/dhcal/dimjc/etc/dimjcd start'" %h
  os.system(cmd)

  
  #try:
  #  cmd="ping -c 1 %s" %h
  #  stdout = subprocess.check_call(cmd, shell=True)
  #  #res.append('%s is ok' % h)
  #except:
  #  res.append('%s is down' % h)
#print "SUMMARY ",res


  #cmd="ssh-copy-id -i .ssh/id_rsa.pub %s" % h
  #os.system(cmd)
  #cmd="ssh pi@%s 'sudo apt-get update '" %h
  #os.system(cmd)
  #cmd="ssh pi@%s 'sudo apt-get -y install libjsoncpp-dev libmysqlclient-dev '" %h
  #os.system(cmd)


  """
