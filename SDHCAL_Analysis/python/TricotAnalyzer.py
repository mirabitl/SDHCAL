#!/usr/bin/env python
import DHCalEventReader as dr
import os,sys
from ROOT import *
import time
#c=TCanvas("test1","test1",800,900)
#c.Draw()
#c.Update()

if len(sys.argv) > 2:
    filen=(sys.argv[1] )
    run =int(sys.argv[2])
else:
    print "Please give a run Number and data file"



try:
    exec("import config_tracking  as config")
except ImportError:
    raise Exception("cannot import")

dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()



config.marlin='m3_analysis.xml'
dher.ParseSteering(config.marlin)
#dher.readGeometry("acqilc/RPC_2008@lyosdhcal10:GEOMETRY","SPS_08_2012");


ar=dr.TricotAnalyzer();


dher.setXdaqShift(24); 
dher.setDropFirstRU(false);
fileList=[filen]

print fileList
time.sleep(5)

dher.registerAnalysis(ar);
#dher.registerAnalysis(ar);
for x in fileList:
    print "================================>",x
    print "================================>",x
    print "================================>",x
    dher.open(x)
   
    dher.readStream(config.nevent)
    
    dher.close()  


rootHandler.writeHistograms("./tricot%d.root" % run)
#a.closeTrees();
