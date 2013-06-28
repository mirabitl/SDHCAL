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
    ifu=int(sys.argv[3] )
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
a.presetParameters()
a.createTrees(config.treeName % (run,ifu))
a.setrebuild(config.rebuild)
a.setuseSynchronised(config.useSynch);
a.setminChambersInTime(config.minChambersInTime);
a.settkMinPoint(config.tkMinPoint);
a.settkExtMinPoint(config.tkExtMinPoint);
a.settkChi2Cut(config.tkChi2Cut);  
a.settkExtChi2Cut(config.tkExtChi2Cut);  

a.settkDistCut(config.tkDistCut);  
a.settkExtDistCut(config.tkExtDistCut);  


fileList=[]

for iseq in range(0,1):
    fileList.append(config.filePath % (run,ifu,iseq))
#fileList=[ config.filePath % (run,0) ,
#           config.filePath % (run,1) ,
#           config.filePath % (run,2) ,
#           config.filePath % (run,3) 
#           ]

print fileList
time.sleep(5)

dher.registerAnalysis(a);
for x in fileList:
    print "================================>",x
    print "================================>",x
    print "================================>",x
    dher.open(x)
   
    dher.readStream(config.nevent)
    
    dher.close()  


rootHandler.writeHistograms(config.rootFilePath % (run,ifu))
a.closeTrees();
