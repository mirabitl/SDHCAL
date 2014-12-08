from ROOT import *
import time
import math
#ch=TChain("showers")





def fitCB(hr,run=0,fo=None):
    x=RooRealVar("x", "Number of Hit", hr.GetXaxis().GetXmin(),hr.GetXaxis().GetXmax())
    l = RooArgList(x)
    data=RooDataHist("data", "Number of Hit data", l, hr)
    frame=x.frame()
    data.plotOn(frame)
    c=TCanvas("Fit results")
    c.cd()
    data.plotOn(frame).Draw()
    mean=RooRealVar("mean", "mean",hr.GetMean(),0.5*hr.GetMean(),2.*hr.GetMean())
    sigma=RooRealVar("sigma", "sigma",hr.GetRMS(),0.5*hr.GetRMS(),2.*hr.GetMean())
    sigma=RooRealVar("sigma", "sigma",hr.GetRMS(),0.5*hr.GetRMS(),2.*hr.GetRMS())
    alpha=RooRealVar("alpha", "alpha", 2., 0.,300)
    nth=RooRealVar("nth", "nth", 2., 0.,300)
    rcb=RooCBShape("rcb","rcb",x,mean,sigma,alpha,nth)
    rcb.fitTo(data)
    rcb.paramOn(frame)
    rcb.plotOn(frame)
    frame.Draw()
    print frame.chiSquare(),mean.getVal(),sigma.getVal(),alpha.getVal(),nth.getVal()
#    if (fo!=None):
#        fo.write("%.1f| %.1f| %.1f | %.1f| %.1f|" % (frame.chiSquare(),mean.getVal(),sigma.getVal(),alpha.getVal(),nth.getVal()))
    if (fo!=None):
        fo.write("%.3f| %.3f|%.3f| %.3f|" % (mean.getVal(),mean.getError(),sigma.getVal(),sigma.getError()))
    c.SaveAs(hr.GetTitle()+"_ROOFIT.png");time.sleep(1)


