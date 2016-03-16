#!/usr/bin/env python
import DHCalEventReader as dr
import os,sys
from ROOT import *
import time
#c=TCanvas("test1","test1",800,900)
#c.Draw()
#c.Update()

if len(sys.argv) > 2:
    energy=float(sys.argv[1] )
    testname=sys.argv[2]
    cut=sys.argv[3]
else:
    print "Please give an energy and a test Name "

mod_name='config_oct2015'


try:
    exec("import %s  as config" % mod_name)
except ImportError:
    raise Exception("cannot import")

dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()



config.marlin='m3_analysis.xml'
dher.ParseSteering(config.marlin)
dher.readGeometry("acqilc/RPC_2008@lyosdhcal10:GEOMETRY",testname);


ar=dr.RawAnalyzer();

a=dr.TrackAnalyzer( dher,rootHandler);
a.presetParameters()
a.createTrees(config.treeName % (energy,0))
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
#dher.setXdaqShift(92); 
#dher.setDropFirstRU(True);
#dher.queryEnergyFiles(energy,True)
dher.queryCutFiles(cut)
time.sleep(5)

dher.processQueriedFiles(config.nevent)
#dher.processQueriedFiles(3000)

rootHandler.writeHistograms(config.rootFilePath % (energy,0))
a.closeTrees();
