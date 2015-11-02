#!/usr/bin/python
import subprocess
host=['lyoilcrpi03','lyoilcrpi04','lyoilcrpi05','lyoilcrpi06','lyoilcrpi07','lyoilcrpi08','lyoilcrpi09','lyoilcrpi10','lyoilcrpi11','lyoilcrpi12','lyoilcrpi14','lyoilcrpi15','lyoilcrpi19','lyoilcrpi17']
res=[]
for h in host:
  try:
    cmd="ping -c 1 %s" %h
    stdout = subprocess.check_call(cmd, shell=True)
    #res.append('%s is ok' % h)
  except:
    res.append('%s is down' % h)
print "SUMMARY ",res
