from ROOT import *
import accessHisto as ah



def fitStrip(run,tdc):
  f=TFile("/tmp/tdcb%d.root" % run)
  myfit=TF1("myfit","gaus",2.,6.)
  c=TCanvas("c","",900,900)
  c.Divide(2,4)
  mean=[]
  res=[]
  for i in range(0,8):
    h=ah.getth1("/run%d/TDC%d/LmAnalysis/hdts%d" % (run,tdc,i))
    h.Rebin(4)
    bmax=0
    ibmax=h.GetMaximumBin()
    bmax=h.GetBinCenter(ibmax)
    c.cd(i+1)
    h.Fit("myfit","Q","",bmax-1.5,bmax+1.5)
    print "%d DIF %d Strip %d Mean %7.3f Width %f " % (run,tdc,i,myfit.GetParameter(1),myfit.GetParameter(2))
    mean.append(myfit.GetParameter(1))
    res.append(myfit.GetParameter(2))
    c.Draw()
    c.Update()
  c.SaveAs("fitTime_%d_%d.pdf" % (run,tdc))
  print mean
  print res
