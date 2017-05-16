#!/usr/bin/env python
import DHCalEventReader as dr
import os,sys
from ROOT import *
import time
#c=TCanvas("test1","test1",800,900)
#c.Draw()
#c.Update()
seqi=0
seql=0

if len(sys.argv) > 1:
    run=int(sys.argv[1] )
else:
    print "Please give a run Number and config file"
    exit(0)
if len(sys.argv) > 3:
    seqi=int(sys.argv[2] )
    seql=int(sys.argv[3] )
    


dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()



#config.marlin='m3_aout2012.xml'
#dher.ParseSteering("xml/m3_oct2015.xml")
#dher.readGeometry("acqilc/RPC_2008@lyosdhcal10:GEOMETRY","SPS_08_2012");
#dher.registerAnalysis("trackAnalysis","./pluggins/");
dher.registerAnalysis("rawAnalysis","./pluggins/");


dher.setXdaqShift(4); 
dher.setDropFirstRU(false);
fileList=[]
filePath="/data/srv02/RAID6/Dome0916/DHCAL_%d_I0_%d.slcio"
#filePath="/data/srv02/RAID6/SPS_10_2015/Other/SO5DHCAL_%d_I9_0.slcio"
#filePath="/data/srv02/RAID6/PS_06_2015/Other/SO3DHCAL_%d_I9_0.slcio"
#80
#fileList=['/data/NAS/October2015/DHCAL_730847_I0_0.slcio']
#fileList=[filePath % run]
for i in range(seqi,seql+1):
    fileList.append(filePath % (run,i))

#40
#fileList=['/data/NAS/October2015/DHCAL_730882_I0_0.slcio']
#20
#fileList=['/data/NAS/October2015/DHCAL_730907_I0_0.slcio']
#fileList=['/data/srv02/RAID6/SPS_10_2015/Other/SO5DHCAL_730886_I9_0.slcio']
#fileList=['/data/srv02/RAID6/SPS_10_2015/Other/SO5DHCAL_730903_I9_0.slcio']

#fileList=['/data/NAS/February2016/DHCAL_732469_I0_0.slcio']
#dher.setXdaqShift(92); 
#dher.setDropFirstRU(True);
print fileList
time.sleep(5)

dher.initJob()
#dher.registerAnalysis(ar);
for x in fileList:
    print "================================>",x
    print "================================>",x
    print "================================>",x
    dher.open(x)
   
    dher.readStream(0)
    
    dher.close()  


rootFilePath="/tmp/trackanalysis%d.root"
rootHandler.writeHistograms(rootFilePath % run)
a.closeTrees();
