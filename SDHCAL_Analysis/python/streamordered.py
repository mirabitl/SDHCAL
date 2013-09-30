#!/usr/bin/python
import DHCalEventReader as dr
import os,sys
if len(sys.argv) > 2:
    run=int(sys.argv[1] )
    nfu=int(sys.argv[2] )
    nseq=int(sys.argv[3] )
else:
    print "Please give a run Number and config file"


fileList=[]

for iseq in range(0,nseq):
    for ifu in range(0,nfu):
        fileList.append("/data/NAS/Results/DHCAL_%d_I%d_%d.slcio" % (run,ifu,iseq))

print fileList

fileOut="/tmp/DHCAL_%d_I9_0.slcio" % run

os.system("rm %s" % fileOut)
#fileOut="/tmp/t27ch.slcio"
  
dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()






dher.ParseSteering("can37.xml")

a=dr.FilterAnalyzer( dher,rootHandler);



a.setuseSynchronized(True);
a.setminChambersInTime(7);
a.setWriting(True)
dher.setDropFirstRU(True);
dher.openOutput(fileOut)
dher.registerAnalysis(a);
#dher.setDropFirstRU(True);
for x in fileList:
    print "================================>",x
    dher.addFile(x)

dher.openFiles()
#print dher.getNumberOfEvents()
for i in range(1,500000):
    dher.readOneEvent(run,i)
#    dher.readStream()
print "closing"
dher.close()  
print "closing output"
dher.closeOutput()
