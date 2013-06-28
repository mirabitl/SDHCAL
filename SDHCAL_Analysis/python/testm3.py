import DHCalEventReader as dr
from ROOT import *
import time
c=TCanvas("test1","test1",800,900)
c.Draw()
c.Update()
#time.sleep(5)  
dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()






dher.ParseSteering("LMCut.xml")
dher.setDropFirstRU(True)

a=dr.ChamberAnalyzer( dher,rootHandler);
a.setrebuild(True)
a.setuseSynchronised(True)
fileList=[ "/data/NAS/Results/DHCAL_713137_I0_0.slcio"]
#           ,"/data/NAS/Results/DHCAL_713167_I1_0.slcio"
#           ,"/data/NAS/Results/DHCAL_713167_I2_0.slcio"
#           ,"/data/NAS/Results/DHCAL_713167_I3_0.slcio"
#          ]

#fileList =["/tmp/test713167.slcio"]

dher.registerAnalysis(a);

dher.initJob();
for x in fileList:
    print "================================>",x
    dher.open(x)
    dher.readStream(1000)
    dher.close()  

rootHandler.writeHistograms("track713137.root")
