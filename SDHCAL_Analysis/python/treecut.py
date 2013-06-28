#!/usr/bin/python
import os,sys,time
#import accessHisto as ah
from ROOT import *

if len(sys.argv) > 1:
    run=int(sys.argv[1])
else:
    print "Please give a file name"

c1=TCanvas("c1","Summary",800,800)
f=TFile("../showers_%d.root" % run)
fo = open('summary_%d.fit' % run, 'w+')
fo.write("| %d " % run);
c1.Divide(1,2)

gStyle.SetOptFit(1)
c1.cd(1)
showers.Draw("fd[0]/TMath::Log(NH0[0]):sqrt(rlambda[0]+rlambda[1])*(lastplan-firstplan)>>h1(200,0,600,70,0,0.07)"," sqrt((rlambda[1]+rlambda[0])/rlambda[2])>0.01 &&(NH[0]<2000&&NH[0]>20) && firstplan<35 && rxm[2]<130. && lastplan>firstplan  &&  sqrt(rlambda[0]+rlambda[1])*(lastplan-firstplan)<600 && fd[0]/TMath::Log(NH0[0])<0.07","CONT")
c1.Modified()
c1.Update()
c1.cd(2)
showers.Draw("fd[0]/TMath::Log(NH0[0]):sqrt(rlambda[0]+rlambda[1])*(lastplan-firstplan)>>h2(200,0,600,70,0,0.07)"," sqrt((rlambda[1]+rlambda[0])/rlambda[2])>0.1 &&(NH[0]<2000&&NH[0]>20) && firstplan<35 && rxm[2]<130. && lastplan>firstplan  &&  sqrt(rlambda[0]+rlambda[1])*(lastplan-firstplan)<600 && fd[0]/TMath::Log(NH0[0])<0.07","CONT")
c1.Modified()
c1.Update()
sok=raw_input("Ok 1/0 \n")

c1.SaveAs("run_cut_%d.svg" % run)
fo.write("|\n");
fo.close()

