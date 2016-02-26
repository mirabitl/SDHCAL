import os
import time
import LMainSlowCtrl
import subprocess

import mc

from threading import Thread

class threadedSendCommand(Thread):
   """
   Class to paralellize command configuration
   """
   def __init__ (self,command):
      """
      Thread inialisation
      """
      Thread.__init__(self)

      self.status = -1
      self.command=command
   def run(self):
      """
      Thread running
      """
      os.system(self.command)
      
         





class StartDaq:
   def __init__(self,mod_name):
      exec("import %s  as config" % mod_name)
      self.slow_=LMainSlowCtrl.DimSlowControl(mod_name)
      
      try:
         self.caenhost_=config.caenhost
         self.caenperiod_=config.caenperiod
      except:
         self.caenhost_=None
         self.caenperiod_=None
      try:
         self.gpiohost_=config.gpiohost
      except:
         self.gpiohost_=None
      try:
         self.ds1820host_=config.ds1820host
         self.ds1820period_=config.ds1820period
      except:
         self.ds1820host_=None
         self.ds1820period_=None
      try:
         self.bmp183host_=config.bmp183host
         self.bmp183period_=config.bmp183period
      except:
         self.bmp183host_=None
         self.bmp183period_=None

      self.slowdb_=os.getenv("SLOWDB")
      if (self.slowdb_== None):
         print "Environment variable SLOWDB must be set (see /etc/difdim.cfg)"
         exit(0)
   def host_status(self):
      if (self.bmp183host != None):
         cmd='ssh pi@'+self.bmp183host_+' "sudo /etc/init.d/dimbmp183d status"'
         print cmd;os.system(cmd)
      if (self.ds1820host != None):
         cmd='ssh pi@'+self.ds1820host__+' "sudo /etc/init.d/dimds1820d status"'
         print cmd;os.system(cmd)
      if (self.gpiohost != None):
         cmd='ssh pi@'+self.gpiohost__+' "sudo /etc/init.d/dimgpiod status"'
         print cmd;os.system(cmd)
      if (self.caenhost != None):
         cmd='ssh acqilc@'+self.caenhost__+' "sudo /etc/init.d/dimcaenhvd status"'
         print cmd;os.system(cmd)

   def host_stop(self):
      if (self.bmp183host != None):
         cmd='ssh pi@'+self.bmp183host_+' "sudo /etc/init.d/dimbmp183d stop"'
         print cmd;os.system(cmd)
      if (self.ds1820host != None):
         cmd='ssh pi@'+self.ds1820host__+' "sudo /etc/init.d/dimds1820d stop"'
         print cmd;os.system(cmd)
      if (self.gpiohost != None):
         cmd='ssh pi@'+self.gpiohost__+' "sudo /etc/init.d/dimgpiod stop"'
         print cmd;os.system(cmd)
      if (self.caenhost != None):
         cmd='ssh acqilc@'+self.caenhost__+' "sudo /etc/init.d/dimcaenhvd stop"'
         print cmd;os.system(cmd)
   
   def host_start(self):
      if (self.bmp183host != None):
         cmd='ssh pi@'+self.bmp183host_+' "sudo /etc/init.d/dimbmp183d start"'
         print cmd;os.system(cmd)
      if (self.ds1820host != None):
         cmd='ssh pi@'+self.ds1820host__+' "sudo /etc/init.d/dimds1820d start"'
         print cmd;os.system(cmd)
      if (self.gpiohost != None):
         cmd='ssh pi@'+self.gpiohost__+' "sudo /etc/init.d/dimgpiod start"'
         print cmd;os.system(cmd)
      if (self.caenhost != None):
         cmd='ssh acqilc@'+self.caenhost__+' "sudo /etc/init.d/dimcaenhvd start"'
         print cmd;os.system(cmd)
   
   def host_restart(self):
      self.host_stop()
      self.host_start()
      self.host_status()
   def Discover(self):
      self.slow_.scandns()
   def CAENHVInitialise(self):
      if (self.caenhost != None):
         self.slow_.CAENHVInitialise(self.slowdb_)
   def CAENHVStartMonitor(self):
      if (self.caenhost != None):
         self.slow_.CAENHVStartMonitor(self.caenhvperiod_)      
   def CAENHVStartRegulation(self):
      if (self.caenhost != None):
         self.slow_.CAENHVStartRegulation(self.caenhvperiod_)      
   def CAENHVStopMonitor(self):
      if (self.caenhost != None):
         self.slow_.CAENHVStopMonitor()
   def CAENHVStopRegulation(self):
      if (self.caenhost != None):
         self.slow_.CAENHVStopRegulation()
   def BMP183Store(self):
      if (self.bmp183host_ !=None):
         self.slow_.BMP183SetPeriod(self.bmp183period_)
         self.slow_.BMP183Store(self.slowdb_[0:self.slowdb_.find(";")])
         
   def DS1820Store(self):
      if (self.ds1820host_ !=None):
         self.slow_.DS1820SetPeriod(self.ds1820period_)
         self.slow_.DS1820Store(self.slowdb_[0:self.slowdb_.find(";")])
         
   def getCurrentRun(self):
      return self.slow_.getCurrentRun()
   def getCurrentHVChannel(self):
      return self.slow_.getCurrentHVChannel()
   def GPIOOpen(self):
      if (self.gpiohost_!=None):
         self.slow_.OpenGPIO();
   def GPIOClose(self):
      if (self.gpiohost_!=None):
         self.slow_.CloseGPIO();
         
   def LVOff(self):
      if (self.gpiohost_!=None):
         self.slow_.DIFOff()
   def LVOn(self):
      if (self.gpiohost_!=None):
         self.slow_.DIFOn()
   def VMEOff(self):
      if (self.gpiohost_!=None):
         self.slow_.VMEOff()
   def VMEOn(self):
      if (self.gpiohost_!=None):
         self.slow_.VMEOn()

   def HVOff(self):
      HT.setGroupsSwitch(GROUPSSWITCH_ALL_OFF)
   def HVOn(self):
      HT.setGroupsSwitch(GROUPSSWITCH_ALL_ON)
   def HVSetVoltage(self,v,ifirst=-1,ilast=-1):
      if (ifirst == -1):
         for i in range(0,51):
            HT.setOutputVoltage(i/8,i%8,v)
      else:
         for ichan in range(ifirst,ilast+1):
            HT.setOutputVoltage(ichan/8,ichan%8,v)
   def HVSwitchOn(self,ichi,icha):
      for i in range(ichi,icha+1):
         HT.setOutputSwitch(i/8,i%8,1)
   def HVSwitchOff(self,ichi,icha):
      for i in range(ichi,icha+1):
         HT.setOutputSwitch(i/8,i%8,0)
   def HVDump(self,ichi,icha):
      for i in range(ichi,icha+1):
         vm=HT.getOutputMeasurementSenseVoltage(i/8,i%8)
         im=HT.getOutputMeasurementCurrent(i/8,i%8)
         print i,vm,im
   def HVGet(self,ichi,icha):
      ic=[]
      vc=[]
      for i in range(ichi,icha+1):
         vm=int(HT.getOutputMeasurementSenseVoltage(i/8,i%8))
         im=abs(int(HT.getOutputMeasurementCurrent(i/8,i%8)*1E6))
         print i,vm,im
         ic.append(im)
         vc.append(vm)
      return [vc,ic]





        
