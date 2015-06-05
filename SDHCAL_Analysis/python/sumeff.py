import accessHisto as ah;from ROOT import *
import os,sys
#f=TFile("/scratch/LMNewAnalyzer_999999.root")
if len(sys.argv) > 1:
    run=int(sys.argv[1] )
f=TFile("/tmp/Monitoring%d.root" % run)
#f=TFile("/dev/shm/LMonitoring.root" )

l={}
for i in range(1,50):
 l[i]=ah.GetEff(i)
 print l[i][6],l[i][7],l[i][8],l[i][10],l[i][9],100*l[i][10]/l[i][9]
