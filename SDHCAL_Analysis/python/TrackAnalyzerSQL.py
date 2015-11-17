#!/usr/bin/env python
import DHCalEventReader as dr
import os,sys
from ROOT import *
import time
#c=TCanvas("test1","test1",800,900)
#c.Draw()
#c.Update()

if len(sys.argv) > 2:
    run=int(sys.argv[1] )
    mod_name=sys.argv[2]
    iseq=int(sys.argv[3] )
else:
    print "Please give a run Number and config file"



try:
    exec("import %s  as config" % mod_name)
except ImportError:
    raise Exception("cannot import")

dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()



config.marlin='m3_aout2012.xml'
dher.ParseSteering(config.marlin)
dher.readGeometry("acqilc/RPC_2008@lyosdhcal10:GEOMETRY","SPS_08_2012");


ar=dr.RawAnalyzer();

a=dr.TrackAnalyzer( dher,rootHandler);
a.presetParameters()
a.createTrees(config.treeName % (run,0))
a.setrebuild(config.rebuild)
a.setuseSynchronised(config.useSynch);
a.setminChambersInTime(config.minChambersInTime);
a.settkMinPoint(config.tkMinPoint);
a.settkExtMinPoint(config.tkExtMinPoint);
a.settkChi2Cut(config.tkChi2Cut);  
a.settkExtChi2Cut(config.tkExtChi2Cut);  

a.settkDistCut(config.tkDistCut);  
a.settkExtDistCut(config.tkExtDistCut);  
dher.registerAnalysis(a);

dher.setXdaqShift(24); 
dher.setDropFirstRU(false);
dher.setXdaqShift(92); 
dher.setDropFirstRU(True);
dher.queryEnergyFiles(40.0,True)
time.sleep(5)

dher.processQueriedFiles(config.nevent)

rootHandler.writeHistograms(config.rootFilePath % (run,0))
a.closeTrees();
