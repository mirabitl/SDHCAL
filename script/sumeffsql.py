import accessHisto as ah;from ROOT import *
import os,sys
import MySQLdb as mdb

#f=TFile("/scratch/LMNewAnalyzer_999999.root")
if len(sys.argv) > 1:
    run=int(sys.argv[1] )
if len(sys.argv) > 4:
    th0=int(sys.argv[2] )
    th1=int(sys.argv[3] )
    th2=int(sys.argv[4] )
else:
    th0=170
    th1=500
    th2=350

con = mdb.connect('localhost', 'acqilc', 'RPC_2008', 'SLOWAVRIL2015');
cur=con.cursor();
f=TFile("/tmp/Monitoring%d.root" % run)
#f=TFile("/dev/shm/LMonitoring.root" )
fl=open("summaryeff%d.txt" % run,"w")
l={}
for i in range(1,50):
 l[i]=ah.GetEff(i)
 print '%d %d %d %d %5.2f %d %d %d %5.2f %5.2f %5.2f' %  (l[i][6],th0,th1,th2,l[i][7],l[i][8],l[i][10],l[i][9],100*l[i][10]/l[i][9],100*l[i][11]/l[i][9],100*l[i][12]/l[i][9])
 fl.write('%d %d %d %d %5.2f %d %d %d %5.2f %5.2f %5.2f\n' %  (l[i][6],th0,th1,th2,l[i][7],l[i][8],l[i][10],l[i][9],100*l[i][10]/l[i][9],100*l[i][11]/l[i][9],100*l[i][12]/l[i][9]))
 cmd= "INSERT INTO THRSCAN(CHANNEL,THR0,THR1,THR2,EFFLOCAL,NLOCAL,NFOUND,NEXT,EFF0,EFF1,EFF2) VALUES(%d,%d,%d,%d,%5.2f,%d,%d,%d,%5.2f,%5.2f,%5.2f)" % (l[i][6],th0,th1,th2,l[i][7],l[i][8],l[i][10],l[i][9],100*l[i][10]/l[i][9],100*(l[i][11])/l[i][9],100*l[i][12]/l[i][9])
 cur.execute(cmd)
fl.close()
con.commit()
con.close()
