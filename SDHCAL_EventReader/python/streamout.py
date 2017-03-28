#!/usr/bin/python
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
    fseq=int(sys.argv[3] )
    lseq=int(sys.argv[4] )
else:
    print "Please give a run Number and config file"



try:
    exec("import %s  as config" % mod_name)
except ImportError:
    raise Exception("cannot import")





#fileOut="/tmp/t27ch.slcio"
  
dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()






dher.ParseSteering(config.marlin)
dher.setXdaqShift(92);
a=dr.FilterAnalyzer( dher,rootHandler);

dher.setCerenkovDifId(config.cerenkovDifId);
dher.setCerenkovOutDifId(config.cerenkovOutDifId);
dher.setCerenkovOutAsicId(config.cerenkovOutAsicId);
dher.setCerenkovOutTimeDelay(config.cerenkovOutTimeDelay);

a.setRebuild(config.rebuild)
a.setuseSynchronized(config.useSynch);
a.setminChambersInTime(config.minChambersInTime);

fileOut=config.fileOut % run
a.setWriting(True)
dher.setDropFirstRU(True);
dher.openOutput(fileOut)
dher.registerAnalysis(a);
#dher.setDropFirstRU(True);



fileList=[]

for iseq in range(fseq,lseq+1):
    fileList.append(config.filePath % (run,iseq,0))


print fileList

#"/tmp/DHCAL_724648_I0_1.slcio"
print fileOut
time.sleep(3)
for x in fileList:
    print "================================>",x
    dher.open(x)
    dher.readStream()
    dher.close()  
dher.endJob()
dher.closeOutput()

#rootHandler.writeHistograms("toto20650.root")
