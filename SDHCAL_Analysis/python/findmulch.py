#!/usr/bin/python
import os,sys,time
import accessHisto as ah
from ROOT import *

if len(sys.argv) > 1:
    run=int(sys.argv[1])
else:
    print "Please give a file name"

#c1=TCanvas()
f=TFile("./tracks_%d.root" % run)
fo = open('chambers_%d.fit' % run, 'w+')
#c1.Divide(1,3);
ok=1

#c1.cd(1)
for i in range(1,51):
    h0=ah.getth1("/TrackCut/Plan_%d_n1" % i )
    h1=ah.getth1("/TrackCut/Plan_%d_n0" % i)
    h2=ah.getth1("/TrackCut/Plan_%d_n2" % i)

    fo.write("%d %6.3f %6.3f %6.3f\n" % (i,h0.GetMean(),h1.GetMean(),h2.GetMean()))


fo.close();
