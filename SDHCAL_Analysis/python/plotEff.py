#!/usr/bin/python
import os,sys,time
import accessHisto as ah
from ROOT import *

if len(sys.argv) > 3:
    run=int(sys.argv[3] )
    chamber=int(sys.argv[2] )
    file_name=sys.argv[1]
else:
    print "Please give a file name and and a chamber and a run"

f=TFile(file_name)
tc=TCanvas("Efficacite","Efficacite",1200,900)
tc.Divide(1,2)
tc.cd(1)
if (chamber>0):
    l=ah.DrawEff(chamber);
    tc.cd(1)
    l[0].Draw("TEXT");
    tc.cd(2)
    l[2].Draw("TEXT");
    tc.SaveAs("Eff_%d_ch%d.pdf" % (chamber,run));    
    time.sleep(1)
    
else:
    nchamber=-1*chamber
    for ich in range(1,nchamber+1):
        print "Chamber ",ich
        tc.cd(1)

        l=ah.DrawEff(ich);
        tc.cd(1)
        l[0].SetStats(False)
        l[0].Draw("TEXT");
        tc.Update()
        tc.cd(2)
        l[2].SetStats(False)

        l[2].Draw("TEXT");
        tc.Update()
        tc.SaveAs("Eff_%d_ch%d.pdf" % (run,ich));    
        time.sleep(1)
