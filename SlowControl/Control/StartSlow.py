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
        except:
           self.ds1820host_=None
        try:
           self.bmp183host_=config.bmp183host
        except:
           self.bmp183host_=None

     
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
        if (self.caenhost != None):
           self.slow_.CAENHVInitiliasise("Start")
    def getCurrentRun(self):
       return self.slow_.getCurrentRun()
    def getCurrentHVChannel(self):
       return self.slow_.getCurrentHVChannel()
        
    def InitialiseWriter(self):
        self.slow_.initialiseWriter(self.directory_)
    def InitialiseZup(self):
       if self.zuphost_!=None:
          self.slow_.initialiseZup(self.zupdevice_,self.zupport_)


    def ChangeState(self,s):
        self.state_=s
        self.slow_.download(self.state_)
    def ChangeRegister(self,r):
        self.register_=r
    def scan(self):
        self.slow_.scan()
    def Print(self):
        self.slow_._print()
    def Initialise(self):
        self.slow_.scan()
        print "USB SCAN completed will sleep 5 second"
        time.sleep(5)
        self.slow_.initialise()
    def Configure(self):
        self.slow_.registerstate(self.register_,self.state_)
        #time.sleep(2)

        self.slow_.configure()
    def Start(self):
        self.slow_.start()

    def StartMonitoring(self,run,ndif):
       if self.monitor_!=None:
          mc.startMonitoring(self.monitor_,8000,"/dev/shm/monitor",ndif,run)
    def StopMonitoring(self):
       if self.monitor_!=None:
          mc.stopMonitoring(self.monitor_,8000)
         
    def Stop(self):
        self.slow_.stop()
    def Pause(self):
        self.slow_.pause()
    def SetThresholds(self,b0,b1,b2):
        self.slow_.setThresholds(b0,b1,b2)
    def Resume(self):
        self.slow_.resume()
    def Destroy(self):
        self.slow_.destroy()
    def Halt(self):
        self.slow_.stop()
        self.slow_.destroy()
    def LVOff(self):
        self.slow_.off()
    def LVOn(self):
        self.slow_.on()
    def zoff(self):
        self.slow_.off()
    def zon(self):
        self.slow_.on()
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





        
