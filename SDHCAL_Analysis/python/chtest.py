#!/usr/bin/env python
import DHCalEventReader as dr
import os,sys

if len(sys.argv) > 2:
    run=int(sys.argv[1] )
    mod_name=sys.argv[2]
else:
    print "Please give a run Number and config file"



try:
    exec("import %s  as config" % mod_name)
except ImportError:
    raise Exception("cannot import")

dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()




dher.ParseSteering(config.marlin)




a=dr.ChamberAnalyzer( dher,rootHandler);
a.setrebuild(config.rebuild)
a.setuseSynchronised(config.useSynch);
a.setminChambersInTime(config.minChambersInTime);
a.settkMinPoint(config.tkMinPoint);
a.settkExtMinPoint(config.tkExtMinPoint);
a.settkChi2Cut(config.tkChi2Cut);  
a.settkExtChi2Cut(config.tkExtChi2Cut);  

a.settkDistCut(config.tkDistCut);  
a.settkExtDistCut(config.tkExtDistCut);  

fileList=[ config.filePath % run ]


dher.registerAnalysis(a);
for x in fileList:
    print "================================>",x
    dher.open(x)
   
    dher.readStream(config.nevent)
    
    dher.close()  


rootHandler.writeHistograms(config.rootFilePath % run)
