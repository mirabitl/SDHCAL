#!/usr/bin/python
import os
os.environ["PATH"] = os.environ["PATH"] +":/sbin"
f=os.popen("sudo lsusb -v -d 0x403: | sed 's/^[ \t]*//;s/[ \t]*$//'")
lines=f.readlines()
devices=[]
info=[]
for x in lines:
 ff=x.split()
 if (len(ff) == 0):
  continue
 if (ff[0] == 'Bus'):
   if (len(info)!=0):
       print info
       devices.append(info)
   info=[]
   info.append(ff[1])
   info.append(ff[3])
   info.append(ff[5])
 if (ff[0] == 'iSerial'):
   info.append(ff[2])
if (len(info)!=0):
    devices.append(info)
for y in devices:
    print "Device found =>"
    bus=y[0]
    dev=y[1].split(":")[0]
    vend=y[2].split(":")[0]
    prod=y[2].split(":")[1]
    ser=y[3]
    command="sudo chmod 666 /dev/bus/usb/"+bus+"/"+dev
    print command
    os.system(command)
    print vend,prod,ser

