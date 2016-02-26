#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import time
import subprocess
import MySQLdb as mdb
from array import array
import sys
import getopt
from ROOT import *
chamber=int(sys.argv[1] )
njours=int(sys.argv[2] )

c1 = TCanvas( 'c1', 'A Simple Graph Example', 200, 10, 1200, 600 )

c1.SetGrid()

con = mdb.connect('localhost', 'acqilc', 'RPC_2008', 'GIFPP2015');
cur=con.cursor();

#cur.execute('SELECT  ID,P,T,UNIX_TIMESTAMP(heure),heure FROM PT ORDER BY ID DESC  WHERE ID> 12000')
#cur.execute('SELECT  ID,P,T,UNIX_TIMESTAMP(heure),heure FROM PT WHERE heure>"2015-05-15 00:00:00" ORDER BY ID DESC LIMIT 16000')
#cur.execute('SELECT  ID,P,T,UNIX_TIMESTAMP(heure),heure FROM PT WHERE now()-heure<432000')
cur.execute('select VMON,IMON,UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(HEURE) from HVMON WHERE  HVCHANNEL=%d AND VMON>1000 AND VMON<8000 AND UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(HEURE)< %d*3600 ORDER BY IDX  DESC ' % (chamber,njours))

rows = cur.fetchall()
n = len(rows)
x, y, z,r,h,rn = array( 'd' ), array( 'd' ), array('d'),array('d'),array('d'),array('d')

t0=None
for row in rows:
      y.append(row[1])
      h.append(row[2])

gr = TGraph( n,h , y )
gr.SetLineColor( 2 )
gr.SetLineWidth( 1 )
gr.SetMarkerColor( 4 )
gr.SetMarkerStyle( 2 ) 
gr.SetTitle( 'Canal HV %d ' % chamber  )
gr.GetXaxis().SetTitle( 'time (s)' )
gr.GetYaxis().SetTitle( 'I (microA)' )
#gr.GetYaxis().SetRangeUser(283,303)
gr.Draw( 'AP' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("imon%d.png" % chamber)
c1.SaveAs("imon%d.root" % chamber)
time.sleep(1)

cur.execute('select TOUT,UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(HEURE) from DS1820MON  WHERE UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(HEURE)< %d*3600 ORDER BY IDX  DESC ' % njours)

rows = cur.fetchall()
n = len(rows)
x, y, z,r,h,rn = array( 'd' ), array( 'd' ), array('d'),array('d'),array('d'),array('d')


for row in rows:
      y.append(row[0]+273.15)
      h.append(row[1])

gr = TGraph( n,h , y )
gr.SetLineColor( 2 )
gr.SetLineWidth( 1 )
gr.SetMarkerColor( 4 )
gr.SetMarkerStyle( 2 ) 
gr.SetTitle( 'T DS1820 Out '  )
gr.GetXaxis().SetTitle( 'time (s)' )
gr.GetYaxis().SetTitle( 'T (C)' )
#gr.GetYaxis().SetRangeUser(283,303)
gr.Draw( 'AL' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("tout.png")
time.sleep(1)
cur.execute('select PRESSURE,TEMPERATURE,UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(HEURE) from PTMON WHERE UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(HEURE)< %d*3600  ORDER BY IDX  DESC  ' % njours)

rows = cur.fetchall()
n = len(rows)
x, y, z,r,h,rn = array( 'd' ), array( 'd' ), array('d'),array('d'),array('d'),array('d')


for row in rows:
      y.append(row[0])
      x.append(row[1])
      h.append(row[2])

gr = TGraph( n,h , y )
gr.SetLineColor( 2 )
gr.SetLineWidth( 1 )
gr.SetMarkerColor( 4 )
gr.SetMarkerStyle( 2 ) 
gr.SetTitle( 'P BMp183 Out '  )
gr.GetXaxis().SetTitle( 'time (s)' )
gr.GetYaxis().SetTitle( 'P (mbar)' )
#gr.GetYaxis().SetRangeUser(283,303)
gr.Draw( 'AL' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("Pout.png")
time.sleep(1)
gr1 = TGraph( n,h , x )
gr1.SetLineColor( 2 )
gr1.SetLineWidth( 1 )
gr1.SetMarkerColor( 4 )
gr1.SetMarkerStyle( 2 ) 
gr1.SetTitle( 'T BMp183 Out '  )
gr1.GetXaxis().SetTitle( 'time (s)' )
gr1.GetYaxis().SetTitle( 'P (mbar)' )
#gr.GetYaxis().SetRangeUser(283,303)
gr1.Draw( 'AL' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("TBMP183.png")
time.sleep(1)
