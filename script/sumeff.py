import accessHisto as ah;from ROOT import *
import os,sys
import MySQLdb as mdb
from array import array

#f=TFile("/scratch/LMNewAnalyzer_999999.root")
fname="/dev/shm/LMonitoring.root"
run=10000
if len(sys.argv) > 1:
    fname=sys.argv[1] 
if len(sys.argv) > 4:
    th0=int(sys.argv[2] )
    th1=int(sys.argv[3] )
    th2=int(sys.argv[4] )
else:
    th0=170
    th1=500
    th2=350

#con = mdb.connect('localhost', 'acqilc', 'RPC_2008', 'SLOWAVRIL2015');
#cur=con.cursor();
f=TFile(fname)
#f=TFile("/dev/shm/LMonitoring.root" )
fl=open("summaryeff%d.txt" % run,"w")
l={}
ch, efl,ef0,ef1,ef2,mul = array( 'd' ), array( 'd' ), array( 'd' ), array( 'd' ), array( 'd' ), array( 'd' )
for i in range(1,9):
 l[i]=ah.GetEff('/TrackPrincipal',i)
 #l[i]=ah.GetEff('/TrackNoCut',i)

 print '%2d %d %d %d %5.2f %6d %6d %6d %5.2f %5.2f %5.2f %5.2f' %  (l[i][6],th0,th1,th2,l[i][7],l[i][8],l[i][10],l[i][9],100*l[i][10]/l[i][9],100*l[i][11]/l[i][9],100*l[i][12]/l[i][9],l[i][13])
 if (l[i][6]<1):
     continue
 if (l[i][6]>51):
     continue
 ch.append(l[i][6])
 mul.append(l[i][13])
 ef2.append(100*l[i][12]/l[i][9])
 ef1.append(100*l[i][11]/l[i][9])
 ef0.append(100*l[i][10]/l[i][9])
 efl.append(l[i][7])
 fl.write('%d %d %d %d %5.2f %d %d %d %5.2f %5.2f %5.2f\n' %  (l[i][6],th0,th1,th2,l[i][7],l[i][8],l[i][10],l[i][9],100*l[i][10]/l[i][9],100*l[i][11]/l[i][9],100*l[i][12]/l[i][9]))
 #cmd= "INSERT INTO THRSCAN(CHANNEL,THR0,THR1,THR2,EFFLOCAL,NLOCAL,NFOUND,NEXT,EFF0,EFF1,EFF2) VALUES(%d,%d,%d,%d,%5.2f,%d,%d,%d,%5.2f,%5.2f,%5.2f)" % (l[i][6],th0,th1,th2,l[i][7],l[i][8],l[i][10],l[i][9],100*l[i][10]/l[i][9],100*(l[i][11])/l[i][9],100*l[i][12]/l[i][9])
 #cur.execute(cmd)
fl.close()
#con.commit()
#con.close()
import time
c1 = TCanvas( 'c1', 'A Simple Graph Example', 200, 10, 700, 500 )
grefl = TGraph(len(ch), ch, efl )
grefl.SetLineColor( 2 )
grefl.SetLineWidth( 4 )
grefl.SetMarkerColor( 2 )
grefl.SetMarkerStyle( 21 )
grefl.SetTitle( 'Efficiency local plan')
grefl.GetXaxis().SetTitle( 'Chamber' )
grefl.GetYaxis().SetTitle( 'Efficacite' )
grefl.Draw( 'AP' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("eflocal.png")
time.sleep(1)
gref0 = TGraph(len(ch), ch, ef0 )
gref0.SetLineColor( 2 )
gref0.SetLineWidth( 4 )
gref0.SetMarkerColor( 2 )
gref0.SetMarkerStyle( 21 )
gref0.SetTitle( 'Efficiency Seuil0 par plan')
gref0.GetXaxis().SetTitle( 'Chamber' )
gref0.GetYaxis().SetTitle( 'Efficacite' )
gref0.Draw( 'AP' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("ef0.png")
time.sleep(1)
gref1 = TGraph(len(ch), ch, ef1 )
gref1.SetLineColor( 2 )
gref1.SetLineWidth( 4 )
gref1.SetMarkerColor( 2 )
gref1.SetMarkerStyle( 21 )
gref1.SetTitle( 'Efficiency Seuil1 par plan')
gref1.GetXaxis().SetTitle( 'Chamber' )
gref1.GetYaxis().SetTitle( 'Efficacite' )
gref1.Draw( 'AP' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("ef1.png")
time.sleep(1)
gref2 = TGraph(len(ch), ch, ef2 )
gref2.SetLineColor( 2 )
gref2.SetLineWidth( 4 )
gref2.SetMarkerColor( 2 )
gref2.SetMarkerStyle( 21 )
gref2.SetTitle( 'Efficiency Seuil2 par plan')
gref2.GetXaxis().SetTitle( 'Chamber' )
gref2.GetYaxis().SetTitle( 'Efficacite' )
gref2.Draw( 'AP' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("ef2.png")
time.sleep(1)
grmul = TGraph(len(ch), ch, mul )
grmul.SetLineColor( 2 )
grmul.SetLineWidth( 4 )
grmul.SetMarkerColor( 2 )
grmul.SetMarkerStyle( 21 )
grmul.SetTitle( 'Multiplicite par plan')
grmul.GetXaxis().SetTitle( 'Chamber' )
grmul.GetYaxis().SetTitle( 'Multiplicite' )
grmul.Draw( 'AP' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("mul.png")
time.sleep(1)
