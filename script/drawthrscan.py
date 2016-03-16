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
cur.execute('select THR0,EFF0,THR1,EFF1,THR2,EFF2 from THRSCAN WHERE CHANNEL=42')


rows = cur.fetchall()
n = len(rows)*3
thr, eff = array( 'd' ), array( 'd' )

t0=None
for row in rows:
      print row[0],row[2],row[3],row[4]
      thr.append((row[0]-90.)/700.)
      eff.append(row[1])
      thr.append((row[2]-98.)/80.)
      eff.append(row[3])
      thr.append((row[4]-98.)/16.3)
      eff.append(row[5])


gr = TGraph( n,thr , eff )
gr.SetLineColor( 2 )
gr.SetLineWidth( 1 )
gr.SetMarkerColor( 4 )
gr.SetMarkerStyle( 2 ) 
gr.SetTitle( 'T BMP183'  )
gr.GetXaxis().SetTitle( 'Threshold (pC)' )
gr.GetYaxis().SetTitle( 'Efficiency' )
#gr.GetYaxis().SetRangeUser(283,303)
gr.Draw( 'AP' )
c1.Update()
c1.GetFrame().SetFillColor(0)
c1.GetFrame().SetBorderSize( 12 )
c1.Modified()
x = input('What are the first 10 perfect squares? ')

#c1.SaveAs("temperature.png")
