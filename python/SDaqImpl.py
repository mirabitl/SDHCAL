#!/usr/bin/python

from PyQt4 import QtGui, QtCore
import sys
import StartDaq
import postelog
import DaqUI
class ImageViewer(QtGui.QMainWindow, DaqUI.Ui_MainWindow): 
    def __init__(self, parent=None):
        super(ImageViewer, self).__init__(parent)
        self.setupUi(self)
        self.daq_=None
        self.isLVOn_=False
        self.connectActions()
    def CreateDaq(self):
        print "On y est"
        name=str(self.LEconfig.text().toAscii())
        print name
        self.daq_=StartDaq.StartDaq(name)
        self.PBCreateDaq.setEnabled(False)
        self.PBDiscover.setEnabled(True)
        self.PBDiscoverDNS.setEnabled(True)
        self.PBDownloadDB.setEnabled(True)
        self.PBInitialiseWriter.setEnabled(True)
    def Discover(self):
        self.daq_.Discover()
        self.PBInitialise.setEnabled(True)
        self.PBDiscover.setEnabled(False)
    def DiscoverDNS(self):
        self.daq_.DiscoverDNS()
        self.PBInitialise.setEnabled(True)
    def DownloadDB(self):
        self.daq_.DownloadDB()
    def InitialiseWriter(self):
        self.daq_.InitialiseWriter()

    def Quit(self):
        exit(0)
    def RestartHost(self):
        if self.daq_!=None:
            self.daq_.host_restart();
    def StartHost(self):
        if self.daq_!=None:
            self.daq_.host_start();
    def StopHost(self):
        if self.daq_!=None:
            self.daq_.host_stop();
    def StartRPI(self):
        if self.daq_!=None:
            self.daq_.rpi_start();
    def StopRPI(self):
        if self.daq_!=None:
            self.daq_.rpi_stop();
    def SynchronizeRPI(self):
        if self.daq_!=None:
            self.daq_.synchronizeRPI();
    def LVOn(self):
        if self.daq_!=None:
            self.daq_.LVOn()
            self.PBLVOff.setEnabled(True)
            self.PBLVOn.setEnabled(False)
            self.isLVOn_=True
    def LVOff(self):
        if self.daq_!=None:
            self.daq_.LVOff()
            self.PBLVOff.setEnabled(False)
            self.PBLVOn.setEnabled(True)
            self.isLVOn_=False
    def SetVoltage(self):
        if self.daq_!=None:
            self.daq_.HVSetVoltage(self.SBVoltage.value(),self.SBFirstHvChannel.value(),self.SBLastHvChannel.value())

    def SwitchHVOn(self):
        if self.daq_!=None:
            self.daq_.HVSwitchOn(self.SBFirstHvChannel.value(),self.SBLastHvChannel.value())
    def SwitchHVOff(self):
        if self.daq_!=None:
            self.daq_.HVSwitchOff(self.SBFirstHvChannel.value(),self.SBLastHvChannel.value())
    def HVDump(self):
        if self.daq_!=None:
            iv=self.daq_.HVGet(self.SBFirstHvChannel.value(),self.SBLastHvChannel.value())
            self.vm_=iv[0]
            self.im_=iv[1]
            print self.vm_
            print self.im_
            self.HSChannel.setEnabled(True)
            self.HSChannel.setMinimum(self.SBFirstHvChannel.value())
            self.HSChannel.setMaximum(self.SBLastHvChannel.value())
    def HVDisplay(self):
        self.LCDChannel.display(self.HSChannel.value())
        self.LCDVoltage.display(self.vm_[self.HSChannel.value()-self.HSChannel.minimum()])
        self.LCDCurrent.display(self.im_[self.HSChannel.value()-self.HSChannel.minimum()])
    
    def Initialise(self):
        if self.daq_!=None and self.isLVOn_:
            self.daq_.Initialise()
            self.PBInitialise.setEnabled(False)
            self.PBConfigure.setEnabled(True)
            self.PBDestroy.setEnabled(True)
            self.PBUpdate.setEnabled(True)
        elif not self.isLVOn_:
            QtGui.QMessageBox.about(self,"LV is OFF","Please put LV On")
            return
        
    
    def Configure(self):
        if self.daq_!=None:
            self.daq_.Configure()
            self.PBConfigure.setEnabled(True)
            self.PBStart.setEnabled(True)
            self.PBDestroy.setEnabled(True)

    def Start(self):
        if self.daq_!=None:
            self.daq_.Start()
            postelog.post(self.daq_.daq_.getCurrentRun(),self.daq_.daq_.getCurrentState())
            self.LCDRun.display(self.daq_.daq_.getCurrentRun())
            self.PBConfigure.setEnabled(False)
            self.PBStart.setEnabled(False)
            self.PBStop.setEnabled(True)
            self.PBDestroy.setEnabled(True)

    def Stop(self):
        if self.daq_!=None:
            self.daq_.Stop()
            self.PBStop.setEnabled(False)
            self.PBStart.setEnabled(True)
            self.PBConfigure.setEnabled(True)
            self.PBDestroy.setEnabled(True)
            
    def Destroy(self):
        if self.daq_!=None:
            self.daq_.Destroy()
            self.PBDestroy.setEnabled(False)
            self.PBConfigure.setEnabled(False)
            self.PBStart.setEnabled(False)
            self.PBStop.setEnabled(False)
            self.PBUpdate.setEnabled(False)
            self.PBInitialise.setEnabled(True)
    def Update(self):
       if self.daq_!=None:
            self.LCDRun.display(self.daq_.daq_.getCurrentRun())
            self.LCDEvent.display(self.daq_.daq_.getCurrentEvent())
            self.daq_.Print()
            self.daq_.daq_.getDifInfo()
            for i in range(0,self.daq_.daq_.seenNumberOfDif()):
                it_id = QtGui.QTableWidgetItem('%d' % self.daq_.daq_.seenId(i))
                self.TWDIF.setItem(i+1, 0,it_id)
                it_slc = QtGui.QTableWidgetItem('%x' % self.daq_.daq_.seenSlc(i))
                self.TWDIF.setItem(i+1, 1,it_slc)
                it_gtc = QtGui.QTableWidgetItem('%d' % self.daq_.daq_.seenGtc(i))
                self.TWDIF.setItem(i+1, 2,it_gtc)
                it_bcid = QtGui.QTableWidgetItem('%ld' % self.daq_.daq_.seenBcid(i))
                self.TWDIF.setItem(i+1, 3,it_bcid)
                it_bytes = QtGui.QTableWidgetItem('%ld' % self.daq_.daq_.seenBytes(i))
                self.TWDIF.setItem(i+1, 4,it_bytes)

                it_host = QtGui.QTableWidgetItem(self.daq_.daq_.seenHost(i))
                self.TWDIF.setItem(i+1, 5,it_host)

                it_state = QtGui.QTableWidgetItem(self.daq_.daq_.seenState(i))
               
                s=self.daq_.daq_.seenState(i)
                if (s.upper().find('FAIL')>=0):
                    it_state.setBackground(QtGui.QColor('red'))
                    QtGui.QMessageBox.about(self,"One DIF  failed" ,"Debug la %d Mon gros !" % self.daq_.daq_.seenId(i))

                self.TWDIF.setItem(i+1, 6,it_state)
            
    def accept(self):
        print "titi"
    def reject(self):
        print "tita"
    def main(self):
        self.show()
        
    def connectActions(self):
        print "On connecte",self.LEconfig.text()
        #QtCore.QObject.connect( self.PBCreateDaq, QtCore.SIGNAL('clicked()'), self.CreateDaq)
        #self.PBCreateDaq.setEnabled(False)
        self.PBCreateDaq.clicked.connect(self.CreateDaq)
        self.PBRestartHost.clicked.connect(self.RestartHost)
        self.PBStartHost.clicked.connect(self.StartHost)
        self.PBStopHost.clicked.connect(self.StopHost)  
        self.PBStartRPI.clicked.connect(self.StartRPI)
        self.PBStopRPI.clicked.connect(self.StopRPI)
        self.PBSynchronizeRPI.clicked.connect(self.SynchronizeRPI)
        self.PBDiscover.clicked.connect(self.Discover)
        self.PBDiscoverDNS.clicked.connect(self.DiscoverDNS)
        self.PBDownloadDB.clicked.connect(self.DownloadDB)
        self.PBInitialiseWriter.clicked.connect(self.InitialiseWriter)
        self.PBLVOn.clicked.connect(self.LVOn)
        self.PBLVOff.clicked.connect(self.LVOff)
        self.PBHVOn.clicked.connect(self.SwitchHVOn)
        self.PBHVOff.clicked.connect(self.SwitchHVOff)
        self.PBSetVoltage.clicked.connect(self.SetVoltage)
        self.PBHVDump.clicked.connect(self.HVDump)
        self.HSChannel.valueChanged.connect(self.HVDisplay)
        self.PBInitialise.clicked.connect(self.Initialise)
        self.PBConfigure.clicked.connect(self.Configure)
        self.PBStart.clicked.connect(self.Start)
        self.PBStop.clicked.connect(self.Stop)
        self.PBDestroy.clicked.connect(self.Destroy)
        self.PBUpdate.clicked.connect(self.Update)
        self.pbQuit.clicked.connect(self.Quit)

    def main(self):
        self.show()
        
if __name__=='__main__':
    app = QtGui.QApplication(sys.argv)
    imageViewer = ImageViewer()
    imageViewer.main()
    app.exec_()
