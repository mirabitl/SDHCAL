#!/usr/bin/python
import DHCalEventReader as dr
import os,sys
from ROOT import *
import time
import glob



if len(sys.argv) > 2:
    testname=sys.argv[1]
    run=int(sys.argv[2])
else:
    print "Please give an energy and a test Name "


stl="/data/srv02/RAID6/%s/Other/SO5DHCAL_%d_*.slcio" % (testname,run)
fileList=glob.glob(stl)

print fileList

time.sleep(5)



  
dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()






dher.ParseSteering('m3_analysis.xml')
dher.readGeometry("acqilc/RPC_2008@lyosdhcal10:GEOMETRY",testname);

dher.setXdaqShift(24);

a=dr.TrackAnalyzer( dher,rootHandler);
a.presetParameters()
rootFilePath="./histos/Tracks%d.root" % run
#treeName="./histos/showers%d.root" % run
#a.createTrees(treeName)
a.setrebuild(False)
a.setuseSynchronised(True)
a.setminChambersInTime(5)
a.settkMinPoint(5)
a.settkExtMinPoint(4)
a.settkChi2Cut(1E-9)
a.settkExtChi2Cut(1E-9)

a.settkDistCut(5)
a.settkExtDistCut(5)
dher.registerAnalysis(a);

a.setWriting(False)
dher.setDropFirstRU(False);


for x in fileList:
    print "================================>",x
    dher.open(x)
    dher.readStream()
    dher.close()  
dher.endJob()

rootHandler.writeHistograms(rootFilePath)
a.closeTrees();

