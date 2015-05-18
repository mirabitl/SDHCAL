from ROOT import *
import rcalib
import rb
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
    return [ frame.chiSquare(),mean.getVal(),sigma.getVal(),alpha.getVal(),nth.getVal()]
def loadfile(ch,run):
    for i in range(0,4):
        ch.Add("./showers_%d_%d.root" % (run,i))

def makeplots(ch,run,fo=None):
    ch.Draw("(nhit[0]+nhit[1]+nhit[2]):sqrt((lambda[0]+lambda[1])/lambda[2])>>htransverse(100,0.,1.,100,0.,%f)" % rcalib.maxhit[run],"","COLZ")
    htransverse.SetTitle("Number_of_Hit_vs_Transverse_ratio_%d" % run)
    ch.Draw("np1>>hnp(50,0.,50.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && rxm[0]>8. && rxm[0]<92. && rxm[1]>8. && rxm[1]<92.  ") 
    hnp.SetTitle("Number_of_Interaction_plans_%d" % run)
    ch.Draw("(nhit[0]+nhit[1]+nhit[2]):fp1>>hfirstplan(50,0.,50.,0.,%f)" % rcalib.maxhit[run],"sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>%d && rxm[0]>8. && rxm[0]<92. && rxm[1]>8. && rxm[1]<92." % rcalib.npcut[run],"PROF") 
    hfirstplan.SetTitle("NumberOfHitVersusFirstInteractionPlan_%d" % run)
    ch.Draw("(nhit[0]+nhit[1]+nhit[2])/(lp1-fp1+1)>>hndens(100,0.,50.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>%d && fp1<%d && rxm[0]>8. && rxm[0]<92. && rxm[1]>8. && rxm[1]<92." % (rcalib.npcut[run],rcalib.fpcut),"") 
    hndens.SetTitle("Hit_density_%d" % run)
    #ch.Draw("fd[0]/log(NH0[0]):n9/(nhit[0]+nhit[1]+nhit[2])>>hfd(100,0.,1.,100,0.01,0.15)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1  && np1>%d && fp1<%d && (nhit[0]+nhit[1]+nhit[2])/(lp1-fp1+1)>%d && rxm[0]>8. && rxm[0]<92. && rxm[1]>8. && rxm[1]<92. " % (rcalib.npcut[run],rcalib.fpcut,rcalib.ndens[run] ),"COLZ")
        # Main Cut 

    theCut="sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 &&  np1>%d  && fp1<%d && (nhit[0]+nhit[1]+nhit[2])/(lp1-fp1+1)>%d && fd[0]/log(NH0[0])<%f && np1*1./(lp1-fp1+1)>0.5 && rxm[0]>8. && rxm[0]<92. && rxm[1]>8. && rxm[1]<92.  " % (rcalib.npcut[run],rcalib.fpcut,rcalib.ndens[run],(1+rcalib.fdcut[run]) )
    
    theCut="sqrt((lambda[0]+lambda[1])/lambda[2])>0.1"

    theCut=theCut+"&& fp1<10 && np1>5 &&  np1*1./(lp1-fp1+1)>0.5 && n9/(nhit[0]+nhit[1]+nhit[2])*fd[0]/log(NH0[0])<0.051"
    theCut=theCut+"&& (eventid<715400 || ((abs(xm[0]-55)<10)&&abs(xm[1]-49)<10))"
    theCut=theCut+"&&(lastplan<40 || rbt<0.15)"


    


    theCut='sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && firstplan<5 && fp1<15 && np1>2 &&  np1*1./(lp1-fp1+1)>0.5 && (eventid<715400 || ((abs(xm[0]-55)<10)&&abs(xm[1]-49)<10))'
    theCut=theCut+"&&(lastplan<40 || rbt<0.15)"

    ch.Draw("fd[0]/log(NH0[0]):n9/(nhit[0]+nhit[1]+nhit[2])>>hfd(100,0.,1.,100,0.01,0.15)",theCut,"COLZ")

    theCut=theCut+"&& (lastplan<=15 || rbs>0.1) && n9/(nhit[0]+nhit[1]+nhit[2])*fd[0]/log(NH0[0])<%g " % rcalib.fd9[run]

    print theCut


    ch.Draw("fd[0]/log(NH0[0]):n9/(nhit[0]+nhit[1]+nhit[2])>>hfd1(100,0.,1.,100,0.01,0.15)",theCut,"COLZ")

    bestX1=0.0469224;  
    bestY1 =0.0417256; 
    bestZ1 =0.252199; 
    bestX2 =-5.85709e-06;  
    bestY2=0.000127489; 
    bestZ2=-4.41272e-05; 
    bestX3=1.10296e-08;  
    bestY3=-6.78472e-08; 
    bestZ3 =-8.52935e-08;

    imadCor="(%g+ %g *(nhit[0]+nhit[1]+nhit[2]) +%g *(nhit[0]+nhit[1]+nhit[2])*(nhit[0]+nhit[1]+nhit[2]))*nhit[0]+(%g+ %g* (nhit[0]+nhit[1]+nhit[2]) +%g* (nhit[0]+nhit[1]+nhit[2])*(nhit[0]+nhit[1]+nhit[2]))* nhit[1]+(%g+ %g *(nhit[0]+nhit[1]+nhit[2]) +%g *(nhit[0]+nhit[1]+nhit[2])*(nhit[0]+nhit[1]+nhit[2]))* nhit[2]" % (bestX1,bestX2,bestX3,bestY1,bestY2,bestY3,bestZ1,bestZ2,bestZ3)

    #imadCor="(.04743+.000014227*(nhit[0]+nhit[1]+nhit[2]))*nhit[0]+(0.0905163-.00004852*(nhit[0]+nhit[1]+nhit[2]))*nhit[1]+(.269448+.000000582*(nhit[0]+nhit[1]+nhit[2])-1.05275e-07*(nhit[0]+nhit[1]+nhit[2])*(nhit[0]+nhit[1]+nhit[2]))*nhit[2]"
    rimadCor="(%g+ %g *(rnhit[0]+rnhit[1]+rnhit[2]) +%g *(rnhit[0]+rnhit[1]+rnhit[2])*(rnhit[0]+rnhit[1]+rnhit[2]))*rnhit[0]+(%g+ %g* (rnhit[0]+rnhit[1]+rnhit[2]) +%g* (rnhit[0]+rnhit[1]+rnhit[2])*(rnhit[0]+rnhit[1]+rnhit[2]))* rnhit[1]+(%g+ %g *(rnhit[0]+rnhit[1]+rnhit[2]) +%g *(rnhit[0]+rnhit[1]+rnhit[2])*(rnhit[0]+rnhit[1]+rnhit[2]))* rnhit[2]" % (bestX1,bestX2,bestX3,bestY1,bestY2,bestY3,bestZ1,bestZ2,bestZ3)

    #rimadCor="(.04743+.000014227*(rnhit[0]+rnhit[1]+rnhit[2]))*rnhit[0]+(0.0905163-.00004852*(rnhit[0]+rnhit[1]+rnhit[2]))*rnhit[1]+(.269448+.000000582*(rnhit[0]+rnhit[1]+rnhit[2])-1.05275e-07*(rnhit[0]+rnhit[1]+rnhit[2])*(rnhit[0]+rnhit[1]+rnhit[2]))*rnhit[2]"

 
#   1  a0           1.64128e-02   4.36609e-04   0.00000e+00   3.98841e-03
#   2  b0           6.94772e-05   3.76344e-07  -0.00000e+00  -1.87843e+01
#   3  a1           6.77964e-01   3.61951e-03  -0.00000e+00   9.69219e-04
#   4  b1          -8.10162e-04   4.42778e-06   0.00000e+00  -1.56137e+00
#   5  a2           2.48824e-01   1.13516e-02   0.00000e+00  -6.42849e-04
#   6  b2          -2.54066e-04   1.50819e-05   1.50819e-05  -4.03742e-01
    #rimadCor="(1.64128e-02+6.94772e-05*(nhit[0]+nhit[1]+nhit[2]))*nhit[0]+(6.77964e-01-8.10162e-04*(nhit[0]+nhit[1]+nhit[2]))*nhit[1]+(2.48824e-01-2.54066e-04*(nhit[0]+nhit[1]+nhit[2])-1.05275e-07*(nhit[0]+nhit[1]+nhit[2])*(nhit[0]+nhit[1]+nhit[2]))*nhit[2]" 
#B1*(0.97638+0.000146*B1)
 
    hfd.SetTitle("FD_vs_NH_%d" % run)
    hfd1.SetTitle("ElectronsRej_FD_vs_NH_%d" % run)
    ch.Draw("nhit[0]>>hnh0(100,0.,%f)" % rcalib.maxhit[run],theCut,"")
    ch.Draw("nhit[1]>>hnh1(100,0.,1000.)",theCut,"")
    ch.Draw("nhit[2]>>hnh2(100,0.,200.)",theCut,"") 
    ch.Draw("nhit[0]+nhit[1]+nhit[2]>>hraw(100,0.,%f)" % rcalib.maxhit[run],theCut,"")
    ch.Draw(imadCor+">>hrawc(50,0.,%f)" % rcalib.maxen[run],theCut,"") 
    ch.Draw("rnhit[0]+rnhit[1]+rnhit[2]>>hred(100,0.,%f)" % rcalib.maxhit[run],theCut,"")
    ch.Draw(rimadCor+">>hredc(500,0.,%f)" % rcalib.maxen[run]  ,theCut,"") 
    #ch.Draw("56.60/1000*(nhit[0]+nhit[1]+nhit[2])*(0.97638+0.000146*(nhit[0]+nhit[1]+nhit[2]))>>henl(50,0.,%f)"  % rcalib.maxen[run],theCut,"")
    ch.Draw("56.60/1000*(nhit[0]+nhit[1]+nhit[2])*(0.95908+1.62233e-04*(nhit[0]+nhit[1]+nhit[2]))>>henl(50,0.,%f)"  % rcalib.maxen[run],theCut,"")
    #ch.Draw("56.39/1000*(nhit[0]+nhit[1]+nhit[2])*(0.9787431 +0.00014211*(nhit[0]+nhit[1]+nhit[2]))>>henl(50,0.,%f)"  % rcalib.maxen[run],theCut,"")
    hnh0.SetTitle("Raw_number_0_%d" % run)
    hnh1.SetTitle("Raw_number_1_%d" % run)
    hnh2.SetTitle("Raw_number_2_%d" % run)
    hraw.SetTitle("Raw_number_of_Hit_%d" % run)
    hrawc.SetTitle("Raw_Energy_with_Imad_weights_%d" % run)
    hred.SetTitle("Reduced_Shower_number_of_hit_%d" % run)
    hredc.SetTitle("Reduced_Shower_Energy_with_Imad_weights_%d" % run)
    henl.SetTitle("Energy_corrected_%d" % run)

    ch.Draw("xm[1]:xm[0]>>hxy(70,10,80.,70,10,80.)",theCut+"&& eventid>715400")
    hxy.SetTitle("august_2012_profile_%d" % run)
    if (fo!=None):
        fo.write("%d|" % hraw.GetEntries())
    hnh=[hnh0,hnh1,hnh2]
    for h in hnh:
        if (fo!=None):
            fo.write("%.1f|" % (h.GetMean()))

    hnl=[hraw,henl,hrawc]
   
    for h in hnl:
        myfit=TF1("myfit","gaus")
        h.Fit("myfit","","",h.GetMean()-h.GetRMS(),h.GetMean()+2*h.GetRMS())
        h.Fit("myfit","","",myfit.GetParameter(1)-2*myfit.GetParameter(2),myfit.GetParameter(1)+2*myfit.GetParameter(2))
       
#        if (fo!=None):
#            fo.write("%.1f| %.1f| %.1f | %.1f|" % (h.GetMean(),myfit.GetParameter(1),h.GetRMS(),myfit.GetParameter(2)))
        if (fo!=None):
            fo.write("%.3f| %.3f| %.3f| %.3f|" % (myfit.GetParameter(1),myfit.GetParError(1),myfit.GetParameter(2),myfit.GetParError(2)))
    
#    far.SetParameter(0,myfit.GetParameter(0))
#    far.SetParLimits(,0.8*myfit.GetParameter(0),1.2*myfit.GetParameter(0))
#    far.SetParameter(1,myfit.GetParameter(1))
#    far.SetParLimits(1,0.8*myfit.GetParameter(1),1.2*myfit.GetParameter(1))
#    far.SetParameter(2,myfit.GetParameter(2))
#    far.SetParLimits(2,0.8*myfit.GetParameter(2),1.2*myfit.GetParameter(2))
#    far.SetParameter(3,2)
#    far.SetParameter(4,1E-5)
#    hraw.Fit("far","","")
    hlist=[]
    hlist.append(htransverse)
    hlist.append(hnp)
    hlist.append(hfirstplan)
    hlist.append(hndens)
    hlist.append(hfd)
    hlist.append(hfd1)
    hlist.append(hnh0)
    hlist.append(hnh1)
    hlist.append(hnh2)
    hlist.append(hraw)
    hlist.append(hrawc)
    hlist.append(hred)
    hlist.append(hredc)
    hlist.append(henl)
    hlist.append(hxy)
    c=TCanvas()
    c.cd()
    for h in hlist:
        gStyle.SetOptFit();
        if (h==hfd or  h==htransverse  or h==hfd1):
            h.Draw("COLZ")
        else:
            h.Draw();
        c.Update();c.SaveAs(h.GetTitle()+".png");

        
    return hlist

