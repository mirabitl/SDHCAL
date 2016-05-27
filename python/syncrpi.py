#!/usr/bin/python
import subprocess
import os
host=['lyoilcrpi03','lyoilcrpi04','lyoilcrpi05','lyoilcrpi06','lyoilcrpi07','lyoilcrpi08','lyoilcrpi09','lyoilcrpi10','lyoilcrpi11','lyoilcrpi12','lyoilcrpi14','lyoilcrpi15','lyoilcrpi19','lyoilcrpi17']
host=['lyoilcrpi03','lyoilcrpi04','lyoilcrpi05','lyoilcrpi06','lyoilcrpi07','lyoilcrpi08','lyoilcrpi09','lyoilcrpi10','lyoilcrpi11','lyoilcrpi12','lyoilcrpi14','lyoilcrpi24','lyoilcrpi19','lyoilcrpi17','lyoilcrpi15']
#host=['lyoilcrpi15']
#host=['lyoilcrpi18']
import time
import argparse

def executeCMD(cmd,lhi):
  lh=[]
  if (lhi=="ALL"):
    lh=host
  else:
    lh.append(lhi)
  for x in lh:
    s=cmd % x
    print "Executing ==>",s
    os.system(s)


parser = argparse.ArgumentParser()

# configure all the actions
grp_action = parser.add_mutually_exclusive_group()
grp_action.add_argument('--restart-dimjc',action='store_true',help='restart dim jc on host --host=name (ALL for all registered)')
grp_action.add_argument('--restart-ljc',action='store_true',help='restart levbdim jc on host --host=name (ALL for all registered)')
grp_action.add_argument('--rsync',action='store_true',help='rsynch /opt/dhcal directory --directory=name (ALL for dim,dimjc,levbdim,lib,etc,bin)  on --host=name (ALL for all registered)')
grp_action.add_argument('--apt-update',action='store_true',help='run apt-get update on host --host=name')
grp_action.add_argument('--apt-upgrade',action='store_true',help='run apt-get upgrade on host --host=name')
grp_action.add_argument('--apt-install',action='store_true',help='run apt-get upinstall --package=name  on host --host=name')
grp_action.add_argument('--poweroff',action='store_true',help='run apt-get upinstall --package=name  on host --host=name')

grp_action.add_argument('--process-kill',action='store_true',help='kill process named --process=name on --host=name')
grp_action.add_argument('--clean',action='store_true',help='kill process named --process=name on --host=name')
# Arguments
parser.add_argument('--directory', action='store', dest='directory',default="ALL",type=str,help='/opt/dhcal/directory to be rsync')
parser.add_argument('--host', action='store', dest='host',default="ALL",help='host name')
parser.add_argument('--package', action='store', type=str,dest='package',default=None,help='package name')
parser.add_argument('--process', action='store', type=str,dest='process',default=None,help='process name')


results = parser.parse_args()

# analyse the command
lcgi={}
r_cmd=None
if (results.restart_dimjc):
  if (results.host==None):
    print 'Please specify the state --host=name'
    exit(0)
  r_cmd="ssh pi@%s 'sudo /opt/dhcal/dimjc/etc/dimjcd stop'"
  executeCMD(r_cmd,results.host)
  r_cmd="ssh pi@%s 'sudo /opt/dhcal/dimjc/etc/dimjcd start'"
  executeCMD(r_cmd,results.host)
  exit(0)

elif(results.restart_ljc):
  if (results.host==None):
    print 'Please specify the state --host=name'
    exit(0)
  r_cmd="ssh pi@%s 'sudo /opt/dhcal/levbdim/bin/jcd stop'"
  srd=executeCMD(r_cmd,results.host)
  r_cmd="ssh pi@%s 'sudo /opt/dhcal/levbdim/bin/jcd start'"
  srd=executeCMD(r_cmd,results.host)
  exit(0)
elif(results.rsync):
  if (results.host==None):
    print 'Please specify the state --host=name'
    exit(0)
  if (results.directory==None):
    print 'Please specify the state --directory=name'
    exit(0)
  ld=[]
  if (results.directory=="ALL"):
    ld=['dimjc','levbdim','dim','lib','bin','etc']
  else:
    ld.append(results.directory)
  for y in ld:
    r_cmd="rsync -axv /opt/dhcal/%s/ %%s:/opt/dhcal/%s/" % (y,y)
    srd=executeCMD(r_cmd,results.host)
  exit(0)
elif(results.apt_update):
  if (results.host==None):
    print 'Please specify the state --host=name'
    exit(0)
  r_cmd="ssh pi@%s 'sudo apt-get update'"
  srd=executeCMD(r_cmd,results.host)
  exit(0)
elif(results.clean):
  if (results.host==None):
    print 'Please specify the state --host=name'
    exit(0)
  r_cmd="ssh pi@%s 'sudo rm /tmp/dimjcPID*.log'"
  srd=executeCMD(r_cmd,results.host)
  exit(0)
elif(results.apt_upgrade):
  if (results.host==None):
    print 'Please specify the state --host=name'
    exit(0)
  r_cmd="ssh pi@%s 'sudo apt-get upgrade'"
  srd=executeCMD(r_cmd,results.host)
  exit(0)
elif(results.apt_install):
  if (results.host==None):
    print 'Please specify the state --host=name'
    exit(0)
  if (results.package==None):
    print 'Please specify the package --package=name'
    exit(0)

  r_cmd="ssh pi@%%s 'sudo apt-get -y install %s'" % results.package
  srd=executeCMD(r_cmd,results.host)
  exit(0)
elif(results.poweroff):
  if (results.host==None):
    print 'Please specify the state --host=name'
    exit(0)

  r_cmd="ssh pi@%s 'sudo poweroff'" 
  srd=executeCMD(r_cmd,results.host)
  exit(0)
elif(results.process_kill):
  if (results.host==None):
    print 'Please specify the state --host=name'
    exit(0)
  if (results.process==None):
    print 'Please specify the process --process=name'
    exit(0)

  r_cmd="ssh pi@%%s 'sudo killall -9 %s'" % results.process
  srd=executeCMD(r_cmd,results.host)
  exit(0)
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
