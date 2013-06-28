#!/usr/bin/python
import os,sys,time
import accessHisto as ah
from ROOT import *

if len(sys.argv) > 1:
    run=int(sys.argv[1])
else:
    print "Please give a file name"

#c1=TCanvas()
f=TFile("./eigen_%d.root" % run)
fo = open('calib_%d.fit' % run, 'w+')
#c1.Divide(1,3);
h0=ah.getth1("/TrackCut/Plan0")
h1=ah.getth1("/TrackCut/Plan1")
h2=ah.getth1("/TrackCut/Plan2")
ok=1

#c1.cd(1)
for i in range(1,51):
    fo.write("%d %6.3f %6.3f %6.3f\n" % (i,h0.GetBinContent(i)/1.5,h1.GetBinContent(i)/0.4,h2.GetBinContent(i)/0.04))


fo.close();
