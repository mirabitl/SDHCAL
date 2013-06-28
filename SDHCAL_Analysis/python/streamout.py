import DHCalEventReader as dr



fileList=[ "/data/NAS/Results/DHCAL_715451_I0_0.slcio"
          ]
#fileList=["/data/online/Results/DHCAL_710456_I0_8.slcio"]



fileOut="/tmp/test713792.slcio"


#fileOut="/tmp/t27ch.slcio"
  
dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()






dher.ParseSteering("/home/acqilc/Slot1_39_MARLIN.xml")

a=dr.FilterAnalyzer( dher,rootHandler);



a.setuseSynchronized(False);
a.setminChambersInTime(5);
a.setWriting(True)
dher.setDropFirstRU(True);
dher.openOutput(fileOut)
dher.registerAnalysis(a);
#dher.setDropFirstRU(True);
for x in fileList:
    print "================================>",x
    dher.open(x)
    dher.readStream()
    dher.close()  

dher.closeOutput()
#rootHandler.writeHistograms("toto20650.root")
