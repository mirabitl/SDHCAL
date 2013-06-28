#!/usr/bin/python
import os,sys,time
import accessHisto as ah
from ROOT import *

if len(sys.argv) > 1:
    run=int(sys.argv[1])
else:
    print "Please give a file name"

c1=TCanvas()
f=TFile("./eigen_%d.root" % run)
fo = open('eigena_%d.fit' % run, 'w+')

for i in range(4,7):
    h=ah.getth1("/AfterValid/hvar%d" % i)
    c1.cd()
    h.Draw()
    c1.Modified()
    c1.Update()
    ok=1
    while (ok==1):
        sxmax=raw_input("Enter xmax \n")
        xmax=float(sxmax)
        if (xmax == 0):
            break
        sxmin=raw_input("Enter xmin \n")
        xmin=float(sxmin)
        myfit=TF1("myfit","gaus",xmin,xmax)

        h.Fit("myfit","","",xmin,xmax)
        c1.Modified()
        c1.Update()
        sok=raw_input("Ok 1/0 \n")
        if (int(sok)==1):
            print "%d %f % f" % (i,myfit.GetParameter(1),myfit.GetParameter(2))
            fo.write("%d %f %f\n" % (i,myfit.GetParameter(1),myfit.GetParameter(2)))
            break
fo.close()

