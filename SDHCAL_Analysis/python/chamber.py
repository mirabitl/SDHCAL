import DHCalEventReader as dr

  
dher=dr.DHCalEventReader();
rootHandler=dr.DCHistogramHandler()






dher.ParseSteering("./steerdz268.xml");

a=dr.ChamberAnalyzer( dher,rootHandler);
a.setuseSynchronised(True);
a.setminChambersInTime(7);
a.settkMinPoint(5);
a.settkExtMinPoint(5);
a.settkChi2Cut(0.5);  
a.settkExtChi2Cut(0.5);  

a.settkDistCut(10.);  
a.settkExtDistCut(10.);  

fileList=("/tmp/DHCAL_20650_I0_0.slcio","/tmp/DHCAL_20650_I1_0.slcio")


dher.registerAnalysis(a);
for x in fileList:
    print "================================>",x
    dher.open(x)
    dher.readStream()
    dher.close()  

rootHandler.writeHistograms("toto20650.root")
