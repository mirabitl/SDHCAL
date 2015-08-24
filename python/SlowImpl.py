#!/usr/bin/python

from PyQt4 import QtGui, QtCore
import sys
import Ldimjc
import postelog
import SlowUI
from ROOT import *
#import mc

class ImageViewer(QtGui.QMainWindow, SlowUI.Ui_MainWindow): 
    def __init__(self, parent=None):
        super(ImageViewer, self).__init__(parent)
        self.setupUi(self)
        self._jc=Ldimjc.DimJobInterface()
        self._slc=None
        self.canvas =None
        self.connectActions()
    def accept(self):
        print "titi"
    def reject(self):
        print "tita"
    def main(self):
        self.show()
    def LoadJSON(self):
        print "On connecte",self.leConfig.text()
        self._jc.loadJSON(str(self.leConfig.text().toAscii()))
    def ScanDNS(self):
        print "On connecte",self.leConfig.text()
    def StartAllHost(self):
        print "On connecte",self.leConfig.text()
    def StopAllHost(self):
        print "On connecte",self.leConfig.text()
    def StartHost(self):
        print "On connecte",self.leConfig.text()
    def StopHost(self):
        print "On connecte",self.leConfig.text()
    def KillProcess(self):
        print "On connecte",self.leConfig.text()
    def QueryProcess(self):
        print "On connecte",self.leConfig.text()
    def RestartProcess(self):
        print "On connecte",self.leConfig.text()
    def DetectGPIO(self):
        print "On connecte",self.leConfig.text()
    def DIFOn(self):
        print "On connecte",self.leConfig.text()
    def DIFOff(self):
        print "On connecte",self.leConfig.text()
    def VMEOn(self):
        print "On connecte",self.leConfig.text()
    def VMEOff(self):
        print "On connecte",self.leConfig.text()
    def HVChannel(self):
        print "On connecte",self.leConfig.text()
    def HVSetAllChannels(self):
        print "On connecte",self.leConfig.text()
    def HVSetI0(self):
        print "On connecte",self.leConfig.text()
    def HVSetV0(self):
        print "On connecte",self.leConfig.text()
    def HVInitialise(self):
        print "On connecte",self.leConfig.text()
    def HVStartMonitoring(self):
        print "On connecte",self.leConfig.text()
    def HVStopMonitoring(self):
        print "On connecte",self.leConfig.text()
    def HVStartRegulation(self):
        print "On connecte",self.leConfig.text()
    def HVStopRegulation(self):
        print "On connecte",self.leConfig.text()
    def BMP183Store(self):
        print "On connecte",self.leConfig.text()
    def BMP183SetPeriod(self):
        print "On connecte",self.leConfig.text()
    def BMP183Read(self):
        print "On connecte",self.leConfig.text()
    def DS1820Store(self):
        print "On connecte",self.leConfig.text()
    def DS1820SetPeriod(self):
        print "On connecte",self.leConfig.text()
    def DS1820Read(self):
        print "On connecte",self.leConfig.text()

    def Quit(self):
        exit(0)
    def connectActions(self):
        print "On connecte",self.leConfig.text()
        #QtCore.QObject.connect( self.PBCreateDaq, QtCore.SIGNAL('clicked()'), self.CreateDaq)
        self.PBLoadJSON.setEnabled(True)
        self.PBLoadJSON.clicked.connect(self.LoadJSON)
        self.pbScanDNS.clicked.connect(self.ScanDNS)
        self.pbQuit.clicked.connect(self.Quit)
        self.PBStartAllHost.clicked.connect(self.StartAllHost)
        self.PBStopAllHost.clicked.connect(self.StopAllHost)
        self.pbStartHost.clicked.connect(self.StartHost)
        self.pbStopHost.clicked.connect(self.StopHost)
        self.pbKillProcess.clicked.connect(self.KillProcess)
        self.pbQueryProcess.clicked.connect(self.QueryProcess)
        self.pbRestartProcess.clicked.connect(self.RestartProcess)
        self.pbDetectGPIO.clicked.connect(self.DetectGPIO)
        self.pbDIFOn.clicked.connect(self.DIFOn)
        self.pbDIFOff.clicked.connect(self.DIFOff)  
        self.pbVMEOn.clicked.connect(self.VMEOn)
        self.pbVMEOff.clicked.connect(self.VMEOff)
        self.lvHVChannel.itemDoubleClicked.connect(self.HVChannel)
        self.pbHVSetAllChannels.clicked.connect(self.HVSetAllChannels)
        self.pbHVSetI0.clicked.connect(self.HVSetI0)
        self.pbHVSetV0.clicked.connect(self.HVSetV0)
        self.pbHVInitialise.clicked.connect(self.HVInitialise)
        self.pbHVStartMonitoring.clicked.connect(self.HVStartMonitoring)
        self.pbHVStopMonitoring.clicked.connect(self.HVStopMonitoring)
        self.pbHVStartRegulation.clicked.connect(self.HVStartRegulation)
        self.pbHVStopRegulation.clicked.connect(self.HVStopRegulation)
        self.pbBMP183Store.clicked.connect(self.BMP183Store)
        self.pbBMP183SetPeriod.clicked.connect(self.BMP183SetPeriod)
        self.pbBMP183Read.clicked.connect(self.BMP183Read)
        self.pbDS1820Store.clicked.connect(self.DS1820Store)
        self.pbDS1820SetPeriod.clicked.connect(self.DS1820SetPeriod)
        self.pbDS1820Read.clicked.connect(self.DS1820Read)
        #self.pbHVSetV0.clicked.connect(self.HVSetV0)
        #sel.PBStartRPI.clicked.connect(self.StartRPI)
        #sel.PBStopRPI.clicked.connect(self.StopRPI)
        #sel.PBSynchronizeRPI.clicked.connect(self.SynchronizeRPI)
        #sel.PBDiscover.clicked.connect(self.Discover)
        #sel.PBDiscoverDNS.clicked.connect(self.DiscoverDNS)
        #sel.PBDownloadDB.clicked.connect(self.DownloadDB)
        #sel.PBInitialiseWriter.clicked.connect(self.InitialiseWriter)
        #sel.PBLVOn.clicked.connect(self.LVOn)
        #sel.PBLVOff.clicked.connect(self.LVOff)
        #sel.PBHVOn.clicked.connect(self.SwitchHVOn)
        #sel.PBHVOff.clicked.connect(self.SwitchHVOff)
        #sel.PBSetVoltage.clicked.connect(self.SetVoltage)
        #sel.PBHVDump.clicked.connect(self.HVDump)
        #self.HSChannel.valueChanged.connect(self.HVDisplay)
        #sel.PBInitialise.clicked.connect(self.Initialise)
        #sel.PBConfigure.clicked.connect(self.Configure)
        #sel.PBStart.clicked.connect(self.Start)
        #sel.PBStop.clicked.connect(self.Stop)
        #sel.PBPause.clicked.connect(self.Pause)
        #sel.PBResume.clicked.connect(self.Resume)
        #sel.PBSetThresholds.clicked.connect(self.SetThresholds)        
        #sel.PBDestroy.clicked.connect(self.Destroy)
        #sel.PBUpdate.clicked.connect(self.Update)
        #self.pbQuit.clicked.connect(self.Quit)
        #self.pbRefresh.clicked.connect(self.Refresh)
        #self.pbGetEff.clicked.connect(self.GetEff)
        #self.tvHistos.doubleClicked.connect(self.histoClick)
    def main(self):
        self.show()
        
if __name__=='__main__':
    app = QtGui.QApplication(sys.argv)
    imageViewer = ImageViewer()
    imageViewer.main()
    app.exec_()
