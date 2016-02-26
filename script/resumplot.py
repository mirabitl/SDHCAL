import accessHisto as ah
from ROOT import *
import os,sys
if len(sys.argv) > 1:
    run=int(sys.argv[1] )
else:
    print "Please give a run Number"

f=TFile("/tmp/Monitoring%d.root" % run)
c=TCanvas("c",'c',600,900)
c.Divide(3,3)
c.cd(1);ah.getth2("/Clusters/Pions/Chamber1/Seuil1").Draw("COLZ")
c.cd(2);ah.getth2("/Clusters/Pions/Chamber2/Seuil1").Draw("COLZ")
c.cd(3);ah.getth2("/Clusters/Pions/Chamber3/Seuil1").Draw("COLZ")
c.cd(4);ah.getth2("/Clusters/Pions/Chamber4/Seuil1").Draw("COLZ")
c.cd(5);ah.getth2("/Clusters/Pions/Chamber5/Seuil1").Draw("COLZ")
c.cd(6);ah.getth2("/Clusters/Pions/Chamber6/Seuil1").Draw("COLZ")
c.cd(7);ah.getth2("/Clusters/Pions/Chamber7/Seuil1").Draw("COLZ")
c.cd(8);ah.getth2("/Clusters/Pions/Chamber8/Seuil1").Draw("COLZ")
c.cd(9);ah.getth2("/Clusters/Pions/Chamber9/Seuil1").Draw("COLZ")
c.SaveAs("BP%d.jpg" % run)
c.SaveAs("BP%d.pdf" % run)
c.Clear()
c.Divide(3,3)
c.cd(1);ah.getth1("/Clusters/hadrons").Fit("gaus","","",30.,350.)
c.cd(2);ah.getth1("/Clusters/pions").Fit("gaus","","",30.,350.)
c.cd(3);ah.getth1("/Clusters/electrons").Fit("gaus","","",30.,350.)
c.cd(4);ah.getth1("/CTag1").Draw()
c.cd(5);ah.getth1("/CTag2").Draw()
c.cd(6);ah.getth1("/CTag3").Draw()
c.cd(7);ah.getth1("/CTag3Notk").Draw()
c.cd(8);ah.getth1("/NoCTag").Draw()
c.cd(9);ah.getth1("/AcquisitionTime").Draw()
c.SaveAs("CK%d.jpg" % run)
c.SaveAs("CK%d.pdf" % run)
