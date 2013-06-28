#!/usr/bin/python
import os,sys,time
import accessHisto as ah
from ROOT import *

if len(sys.argv) > 2:
    chamber=int(sys.argv[2] )
    file_name=sys.argv[1]
else:
    print "Please give a file name and and a chamber"

c1=TCanvas()
f=TFile(file_name)
if (chamber>0):
    for x,y in ah.getDifList(chamber).iteritems():
        hm=ah.tagDIFHits(y,x);c1.cd();hm.Draw();c1.Update();time.sleep(2)
else:
    nchamber=-1*chamber
    for ich in range(1,nchamber+1):
        print "Chamber ",ich
        for x,y in ah.getDifList(ich).iteritems():
            hm=ah.tagDIFHits(y,x);c1.cd();hm.Draw();c1.Update();time.sleep(2)
