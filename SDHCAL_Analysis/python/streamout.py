#!/usr/bin/python
import DHCalEventReader as dr



fileList=[
"/data/NAS/Results/DHCAL_716308_I1_0.slcio"]
#"/data/NAS/Results/DHCAL_716308_I0_0.slcio",
#"/data/NAS/Results/DHCAL_716308_I1_0.slcio",
#"/data/NAS/Results/DHCAL_716308_I2_0.slcio",
#"/data/NAS/Results/DHCAL_716308_I4_0.slcio"]
#fileList=["/data/online/Results/DHCAL_710456_I0_8.slcio"]



fileOut="/tmp/DHCAL_716308_I1_0.slcio"


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
    dher.open(x)
    dher.readStream()
    dher.close()  
dher.endJob()
dher.closeOutput()

#rootHandler.writeHistograms("toto20650.root")
