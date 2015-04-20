import os
import time
import LSDHCALDimCtrl
import subprocess
import biblioSNMP as HT


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
        self.host_=config.host
        self.ccc_=config.ccc
        self.db_=config.db
        self.writer_=config.writer
        self.dim_='/opt/dhcal/dim/'
        self.bin_='/opt/dhcal/bin/'
        self.lib_='/opt/dhcal/lib/'
        self.daq_=LSDHCALDimCtrl.DimDaqControl(mod_name)
        self.state_=config.state
        self.register_=config.register
        self.directory_=config.directory
        try:
           self.zuphost_=config.zuphost
           self.zupdevice_=config.zupdevice
           self.zupport_=config.zupport
        except:
           self.zuphost_=None
           self.zupdevice_=None
           self.zupport_=None
           
    def addHost(self,h):
        self.host_.append(h)
    def addHostRange(self,name,first,last):
        for i in range(first,last+1):
            self.host_.append(name+'%.2d' % i)
        self.host_
    def synchronizeRPI(self):
        for h in self.host_:
            cmd='rsync -axv '+self.dim_+' pi@'+h+':'+self.dim_
            print cmd;os.system(cmd)
            cmd='rsync -axv '+self.lib_+' pi@'+h+':'+self.lib_
            print cmd;os.system(cmd)
            cmd='rsync -axv '+self.bin_+' pi@'+h+':'+self.bin_
            print cmd;os.system(cmd)
    def host_status(self):
         for h in self.host_:
            cmd='ssh pi@'+h+' "sudo /etc/init.d/dimdifd status"'
            tcm=threadedSendCommand(cmd)
            tcm.start();
            print cmd;
            #os.system(cmd)
         cmd='ssh pi@'+self.ccc_+' "sudo /etc/init.d/dimcccd status"'
         print cmd;os.system(cmd)
         cmd='ssh acqilc@'+self.db_+' "sudo /etc/init.d/dimdbd status"'
         print cmd;os.system(cmd)
         cmd='ssh acqilc@'+self.writer_+' "sudo /etc/init.d/dimwriterd status"'
         print cmd;os.system(cmd)
         if self.zuphost_!=None:
            cmd='ssh pi@'+self.zuphost_+' "sudo /etc/init.d/dimzupd status"'
            print cmd;os.system(cmd)

    def host_stop(self):
         for h in self.host_:
            cmd='ssh pi@'+h+' "sudo /etc/init.d/dimdifd stop"'
            tcm=threadedSendCommand(cmd)
            tcm.start();
            print cmd;

            #print cmd;os.system(cmd)
         cmd='ssh pi@'+self.ccc_+' "sudo /etc/init.d/dimcccd stop"'
         print cmd;os.system(cmd)
         cmd='ssh acqilc@'+self.db_+' "sudo /etc/init.d/dimdbd stop"'
         print cmd;os.system(cmd)
         cmd='ssh acqilc@'+self.writer_+' "sudo /etc/init.d/dimwriterd stop"'
         print cmd;os.system(cmd)
         if self.zuphost_!=None:
            cmd='ssh pi@'+self.zuphost_+' "sudo /etc/init.d/dimzupd stop"'
            print cmd;os.system(cmd)

    def host_start(self):
         for h in self.host_:
            cmd='ssh pi@'+h+' "sudo /etc/init.d/dimdifd start"'
            tcm=threadedSendCommand(cmd)
            tcm.start();
            print cmd;
            
            #print cmd;os.system(cmd)
         cmd='ssh pi@'+self.ccc_+' "sudo /etc/init.d/dimcccd start"'
         print cmd;os.system(cmd)
         cmd='ssh acqilc@'+self.db_+' "sudo /etc/init.d/dimdbd start"'
         print cmd;os.system(cmd)
         cmd='ssh acqilc@'+self.writer_+' "sudo /etc/init.d/dimwriterd start"'
         print cmd;os.system(cmd)
         if self.zuphost_!=None:
            cmd='ssh pi@'+self.zuphost_+' "sudo /etc/init.d/dimzupd start"'
            print cmd;os.system(cmd)

    def rpi_stop(self):
         for h in self.host_:
            cmd='ssh pi@'+h+' "sudo /etc/init.d/dimdifd stop"'
            tcm=threadedSendCommand(cmd)
            tcm.start();
            print cmd;


    def rpi_start(self):
         for h in self.host_:
            cmd='ssh pi@'+h+' "sudo /etc/init.d/dimdifd start"'
            tcm=threadedSendCommand(cmd)
            tcm.start();
            print cmd;
            

    def host_restart(self):
        self.host_stop()
        self.host_start()
        self.host_status()
    def Discover(self):
        self.daq_.scandns()
        self.daq_.download(self.state_)
        self.daq_.initialiseWriter(self.directory_)
        self.InitializeZup()
    def DiscoverDNS(self):
        self.daq_.scandns()
        self.InitializeZup()
    def DownloadDB(self):
        self.daq_.download(self.state_)
        
    def InitialiseWriter(self):
        self.daq_.initialiseWriter(self.directory_)
    def InitialiseZup(self):
       if self.zuphost_!=None:
          self.daq_.initialiseZup(self.zupdevice_,self.zupport_)


    def ChangeState(self,s):
        self.state_=s
        self.daq_.download(self.state_)
    def ChangeRegister(self,r):
        self.register_=r
    def scan(self):
        self.daq_.scan()
    def Print(self):
        self.daq_._print()
    def Initialise(self):
        self.daq_.scan()
        print "USB SCAN completed will sleep 5 second"
        time.sleep(5)
        self.daq_.initialise()
    def Configure(self):
        self.daq_.registerstate(self.register_,self.state_)
        #time.sleep(2)

        self.daq_.configure()
    def Start(self):
        self.daq_.start()
    def Stop(self):
        self.daq_.stop()
    def Destroy(self):
        self.daq_.destroy()
    def Halt(self):
        self.daq_.stop()
        self.daq_.destroy()
    def LVOff(self):
        self.daq_.off()
    def LVOn(self):
        self.daq_.on()
    def zoff(self):
        self.daq_.off()
    def zon(self):
        self.daq_.on()
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





        
