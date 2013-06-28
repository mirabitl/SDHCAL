#!/usr/bin/python
import os,sys,time
import accessHisto as ah
from ROOT import *

if len(sys.argv) > 1:
    run=int(sys.argv[1])
else:
    print "Please give a file name"

c1=TCanvas()
f=TFile("./eigen_%d.root" % run)
fo = open('summary_%d.fit' % run, 'w+')
fo.write("| %d " % run);
histname=["hen","hnt","/AfterValidAll/hrnh0","/AfterValidAll/hrnh1","/AfterValidAll/hrnh2","/AfterValidAll/hFD"]
for name in histname:
    h=ah.getth1(name)
    c1.cd()
    h.Draw()
    c1.Modified()
    c1.Update()
    ok=1
    myfit=TF1("myfit","gaus")
    h.Fit("myfit","","")
    h.Fit("myfit","","",myfit.GetParameter(1)-2*myfit.GetParameter(2),myfit.GetParameter(1)+2*myfit.GetParameter(2))
    c1.Modified()
    c1.Update()
    #sok=raw_input("Ok 1/0 \n")

    print "%s %f % f %f %f " % (name,myfit.GetParameter(1),myfit.GetParameter(2),h.GetMean(),h.GetRMS())
    #fo.write("| %.1f | %.1f  " % (myfit.GetParameter(1),myfit.GetParameter(2)))
    if (h.GetMean()>5.):
        fo.write("| %.1f | %.1f  " % (myfit.GetParameter(1),myfit.GetParameter(2)))
    else:
        fo.write("| %.3f | %.3f  " % (myfit.GetParameter(1),myfit.GetParameter(2)))
    #    fo.write("| %.1f | %.1f  " % (h.GetMean(),h.GetRMS()))
    #else:
    #    fo.write("| %.1f | %.1f  " % (h.GetMean()*100,h.GetRMS()*100))
    sok=raw_input("Ok 1/0 \n")



histname1=["hntpl0","hntpl1","hntpl2"]
f.cd()
for name in histname1:
    print name
    h=ah.getth1(name)
    c1.cd()
    h.Draw()
    c1.Modified()
    c1.Update()
    ok=1
    myfit=TF1("myfit","pol0")
    h.Fit("myfit","","",35,50)
    c1.Modified()
    c1.Update()
       

    print "%s %f  " % (name,myfit.GetParameter(0))
    fo.write("| %.1f " % (myfit.GetParameter(0)))
    
    #sok=raw_input("Ok 1/0 \n")


fo.write("|\n");
fo.close()

