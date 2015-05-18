import accessHisto as ah;from ROOT import *
import os,sys
#f=TFile("/scratch/LMNewAnalyzer_999999.root")
if len(sys.argv) > 2:
    run=int(sys.argv[1] )
    hv=sys.argv[2]
f=TFile("/tmp/LMNewAnalyzer_%d.root" % run)

l={}
for i in range(1,50):
 l[i]=ah.GetEff(i)
 #print run,hv,l[i][6],l[i][7],l[i][8],l[i][10],l[i][9],100*l[i][10]/l[i][9]
 cmd ='INSERT INTO MONITOREFF(RUN,HV,CHANNEL,EFFLOCAL,NLOCAL,NFOUND,NEXT,EFFGLOBAL) VALUES(%d,%f,%d,%f,%d,%d,%d,%f);' % ( int(run),float(hv),l[i][6],float(l[i][7]),int(l[i][8]),int(l[i][10]),int(l[i][9]),100*l[i][10]/l[i][9])
 print cmd
