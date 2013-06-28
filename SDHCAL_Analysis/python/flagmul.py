#!/usr/bin/python
import os,sys,time
import accessHisto as ah
from ROOT import *

if len(sys.argv) > 1:
    run=int(sys.argv[1])
else:
    print "Please give a file name"

c1=TCanvas("Eff","Eff",800,900)
f=TFile("./tracks_%d.root" % run)
fo = open('efficiency_%d.fit' % run, 'w+')

c1.Divide(1,3);
alleff=0
allmul=0.
np=0;
for i in range(1,51):
    hfound=ah.getth2("/TrackCut/Plan_%d_found" % i )
    hext=ah.getth2("/TrackCut/Plan_%d_ex" % i)
    hmul=ah.getth2("/TrackCut/Plan_%d_mul" % i)
    hmul0=ah.getth2("/TrackCut/Plan_%d_mul0" % i)
    hmul1=ah.getth2("/TrackCut/Plan_%d_mul1" % i)
    hmul2=ah.getth2("/TrackCut/Plan_%d_mul2" % i)


    meff=0
    mmul=0
    mmul0=0
    mmul1=0
    mmul2=0
    mext=0
    effmax=0;
    for ib in range(2,8):
        for jb in range(2,8):
            if (hext.GetBinContent(ib,jb)<100): 
                continue;
            if (hfound.GetBinContent(ib,jb)<50): 
                continue;
            theeff=hfound.GetBinContent(ib,jb)/hext.GetBinContent(ib,jb)
            if (theeff>effmax):
                effmax=theeff;

    for ib in range(2,8):
        for jb in range(2,8):
            meff=meff+hfound.GetBinContent(ib,jb)
            mmul=mmul+hmul.GetBinContent(ib,jb)
            mmul0=mmul0+hmul0.GetBinContent(ib,jb)
            mmul1=mmul1+hmul1.GetBinContent(ib,jb)
            mmul2=mmul2+hmul2.GetBinContent(ib,jb)
            mext=mext+hext.GetBinContent(ib,jb)
    print " on trouve", mext,meff,mmul,mmul0,mmul1,mmul2,"===>",effmax,meff/(mext+1),mmul/(meff+1),mmul0/(meff+1),mmul1/(meff+1),mmul2/(meff+1)
    hfound.SetMarkerSize(2.9)
    hext.SetMarkerSize(2.9)
    hmul.SetMarkerSize(2.9)
    hmul.Divide(hfound);
    hfound.Divide(hext)
    c1.cd(1)
    hext.Draw("TEXT");
    c1.cd(2)
    hfound.Draw("TEXT");
    c1.cd(3)
    hmul.Draw("TEXT");
    c1.Modified()
    c1.Update()
    ok=1
    sok=raw_input("Ok 1/0 \n")
    c1.SaveAs("EFf_%d_p%d.pdf" % (run,i))
    meff=0
    mmul=0
    mext=0
    for ib in range(2,8):
        for jb in range(2,8):
            if (hfound.GetBinContent(ib,jb)>0):
                fo.write("%d %d %d %6.3f %6.3f %6.3f\n" % (i,ib,jb,hext.GetBinContent(ib,jb),hfound.GetBinContent(ib,jb),hmul.GetBinContent(ib,jb)))
            meff=meff+hfound.GetBinContent(ib,jb)
            mmul=mmul+hmul.GetBinContent(ib,jb)
            mext=mext+hext.GetBinContent(ib,jb)

    meff=meff/64.
    mmul=mmul/64
    if (meff>0.8):
        alleff=alleff+meff
        allmul=allmul+mmul
        np=np+1
    #fo.write("Summary %d %f %f\n" % (i,meff,mmul))
#c1.cd(1)
#for i in range(1,51):
#    fo.write("%d %6.3f %6.3f %6.3f\n" % (i,h0.GetBinContent(i)/1.5,h1.GetBinContent(i)/0.4,h2.GetBinContent(i)/0.04))
    #print alleff/np,allmul/np,np

fo.close();
