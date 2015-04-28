import accessHisto as ah;from ROOT import *
#f=TFile("/scratch/LMNewAnalyzer_999999.root")
f=TFile("toto.root")

l={}
for i in range(1,51):
 l[i]=ah.GetEff(i)
 print l[i][6],l[i][7],l[i][8],l[i][10],l[i][9],100*l[i][10]/l[i][9]

