#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import time
import subprocess
import MySQLdb as mdb
from array import array

from ROOT import *
c1 = TCanvas( 'c1', 'A Simple Graph Example', 200, 10, 1200, 600 )

c1.SetGrid()

con = mdb.connect('localhost', 'acqilc', 'RPC_2008', 'SLOWAVRIL2015');
cur=con.cursor();

#cur.execute('SELECT  ID,P,T,UNIX_TIMESTAMP(heure),heure FROM PT ORDER BY ID DESC  WHERE ID> 12000')
cur.execute('SELECT  ID,P,T,UNIX_TIMESTAMP(heure),heure FROM PT WHERE heure>"2015-05-15 00:00:00" ORDER BY ID DESC LIMIT 16000')
#cur.execute('SELECT  ID,P,T,UNIX_TIMESTAMP(heure),heure FROM PT WHERE now()-heure<432000')


rows = cur.fetchall()
n = len(rows)
x, y, z,r,h,rn = array( 'd' ), array( 'd' ), array('d'),array('d'),array('d'),array('d')

t0=None
for row in rows:
      print row[4]
      x.append(row[0])
      y.append(row[1])
      z.append(row[2])
      r.append(row[2]/(row[1]+273.15))
    # variation on slot 47
      rn.append((row[2]/(row[1]+273.15)/3.2862-1)*7200) 
      if (t0 == None):
        t0=row[3]
      h.append((row[3]-t0)/3600.)

gr = TGraph( n,h , y )
gr.SetLineColor( 2 )
gr.SetLineWidth( 1 )
gr.SetMarkerColor( 4 )
gr.SetMarkerStyle( 2 ) 
gr.SetTitle( 'T BMP183'  )
gr.GetXaxis().SetTitle( 'time (hour)' )
gr.GetYaxis().SetTitle( 'T Celsius' )
#gr.GetYaxis().SetRangeUser(283,303)
gr.Draw( 'AL' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("temperature.png")
time.sleep(3)
gr1 = TGraph( n, h, z )
gr1.SetLineColor( 2 )
gr1.SetLineWidth( 1 )
gr1.SetMarkerColor( 4 )
gr1.SetMarkerStyle( 2 ) 
gr1.SetTitle( 'P BMP183'  )
gr1.GetXaxis().SetTitle( 'time (hour)' )
gr1.GetYaxis().SetTitle( 'P mbar' )
#gr1.GetYaxis().SetRangeUser(920,1000)

gr1.Draw( 'AL' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("pression.png")
time.sleep(3)
gr2 = TGraph( n, h, r )
gr2.SetLineColor( 2 )
gr2.SetLineWidth( 1 )
gr2.SetMarkerColor( 4 )
gr2.SetMarkerStyle( 2 ) 
gr2.SetTitle( 'P/T BMP183'  )
gr2.GetXaxis().SetTitle( 'time (hour)' )
gr2.GetYaxis().SetTitle( 'P/T mbar/K' )
#gr2.GetYaxis().SetRangeUser(3.15,3.35)

gr2.Draw( 'AL' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("povert.png")

time.sleep(3)
gr3 = TGraph( n, h, rn )
gr3.SetLineColor( 2 )
gr3.SetLineWidth( 1 )
gr3.SetMarkerColor( 4 )
gr3.SetMarkerStyle( 2 ) 
gr3.SetTitle( '$\Delta$V on slot 47 '  )
gr3.GetXaxis().SetTitle( 'time (hour)' )
gr3.GetYaxis().SetTitle( '$\Delta$V on slot 47 (V)' )
#gr2.GetYaxis().SetRangeUser(3.15,3.35)

gr3.Draw( 'AL' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
c1.SaveAs("povertrel.png")
time.sleep(3)
