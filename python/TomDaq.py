import os
import time
import LSDHCALDimCtrl
import subprocess
import LTMVDB



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
      
         





class TomDaq:
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
        self.rundb_=LTMVDB.TmvAccessSql()
        self.rundb_.connect(config.mysqlaccount)


           
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
    def DiscoverDNS(self):
        self.daq_.scandns()

    def DownloadDB(self):
        self.daq_.download(self.state_)
    def InitialiseWriter(self):
        self.daq_.initialiseWriter(self.directory_)
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
        time.sleep(2)
        print " registerd done"
        self.daq_.configure()
    def Start(self):

        self.daq_.start()
        print "Run",self.daq_.getCurrentRun(),"is started"
        self.rundb_.initRunTable(self.daq_.getCurrentRun())
    def Stop(self):
        print "Run",self.daq_.getCurrentRun(),"will stop"
        self.daq_.stop()
        self.rundb_.endRunTable()
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
   





        
