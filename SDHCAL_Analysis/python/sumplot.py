from ROOT import *

#ch=TChain("showers")
def loadfile(ch,run):
    for i in range(0,4):
        ch.Add("../showers_%d_%d.root" % (run,i))

def makeplots(ch,fo=None):
    ch.Draw("(nhit[0]+nhit[1]+nhit[2]):sqrt((lambda[0]+lambda[1])/lambda[2])>>htransverse(100,0.,1.,100,0.,2000.)","","COLZ")
    htransverse.SetTitle("Number of Hit vs Transverse ratio")
    ch.Draw("np1>>hnp(50,0.,50.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50") 
    hnp.SetTitle("Number of Interaction plans")
    ch.Draw("NH[0]:fp1>>hfirstplan(50,0.,50.,0.,2000.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15","PROF") 
    hfirstplan.SetTitle("Number of hit versus first interaction plan")
    ch.Draw("NH[0]/(lp1-fp1+1)>>hndens(100,0.,50.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15 && fp1<15","") 
    hndens.SetTitle("Hit density")
    ch.Draw("fd[0]/log(NH0[0]):NH[0]>>hfd(200,0.,2000.,100,0.01,0.09)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15 && fp1<15 && NH[0]/(lp1-fp1+1)>7","") 
    hfd.SetTitle("FD vs NH")
    ch.Draw("nhit[0]>>hnh0(200,0.,2000.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15 && fp1<15 && NH[0]/(lp1-fp1+1)>7 && fd[0]/log(NH0[0])<0.05","")
    ch.Draw("nhit[1]>>hnh1(100,0.,1000.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15 && fp1<15 && NH[0]/(lp1-fp1+1)>7 && fd[0]/log(NH0[0])<0.05","")
    ch.Draw("nhit[2]>>hnh2(100,0.,200.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15 && fp1<15 && NH[0]/(lp1-fp1+1)>7 && fd[0]/log(NH0[0])<0.05","") 
    ch.Draw("nhit[0]+nhit[1]+nhit[2]>>hraw(200,0.,2000.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15 && fp1<15 && NH[0]/(lp1-fp1+1)>7 && fd[0]/log(NH0[0])<0.05","")
    ch.Draw("nhit[0]+nhit[1]+2*nhit[2]>>hrawc(200,0.,2000.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15 && fp1<15 && NH[0]/(lp1-fp1+1)>7 && fd[0]/log(NH0[0])<0.05","") 
    ch.Draw("rnhit[0]+rnhit[1]+rnhit[2]>>hred(200,0.,2000.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15 && fp1<15 && NH[0]/(lp1-fp1+1)>7 && fd[0]/log(NH0[0])<0.05","")
    ch.Draw("rnhit[0]+rnhit[1]+2*rnhit[2]>>hredc(200,0.,2000.)","sqrt((lambda[0]+lambda[1])/lambda[2])>0.1 && fp1<50 && np1>15 && fp1<15 && NH[0]/(lp1-fp1+1)>7 && fd[0]/log(NH0[0])<0.05","") 
    hnh0.SetTitle("Raw number 0")
    hnh1.SetTitle("Raw number 1")
    hnh2.SetTitle("Raw number 2")
    hraw.SetTitle("Raw number ")
    hrawc.SetTitle("Raw number weight 2 on NH2")
    hred.SetTitle("Reduced number ")
    hredc.SetTitle("Reduced number weight 2 on NH2")
    
    hnl=[hnh0,hnh1,hnh2,hraw,hrawc,hred,hredc]
    for h in hnl:
        myfit=TF1("myfit","gaus")
        h.Fit("myfit","","",h.GetMean()-h.GetRMS(),h.GetMean()+2*h.GetRMS())
        h.Fit("myfit","","",myfit.GetParameter(1)-2*myfit.GetParameter(2),myfit.GetParameter(1)+2*myfit.GetParameter(2))
        if (fo!=None):
            fo.write("%.1f| %.1f| %.1f | %.1f|" % (h.GetMean(),h.GetRMS(),myfit.GetParameter(1),myfit.GetParameter(2)))
    

    hlist=[]
    hlist.append(htransverse)
    hlist.append(hnp)
    hlist.append(hfirstplan)
    hlist.append(hndens)
    hlist.append(hfd)
    hlist.append(hnh0)
    hlist.append(hnh1)
    hlist.append(hnh2)
    hlist.append(hraw)
    hlist.append(hrawc)
    hlist.append(hred)
    hlist.append(hredc)

    return hlist
