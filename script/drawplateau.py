#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import time
import subprocess
import MySQLdb as mdb
from array import array

from ROOT import *
c1 = TCanvas( 'c1', 'A Simple Graph Example', 200, 10, 700, 500 )

c1.SetGrid()

con = mdb.connect('localhost', 'acqilc', 'RPC_2008', 'SLOWAVRIL2015');
cur=con.cursor();
for ich in range(1,50):
    cur.execute("SELECT  HV,CHANNEL,EFFGLOBAL from MONITOREFF WHERE CHANNEL=%d AND RUN!=728103 ORDER BY CHANNEL,HV" % ich)


    rows = cur.fetchall()
    n = len(rows)
    x, y = array( 'd' ), array( 'd' )
    for row in rows:
        print row
        x.append(row[0])
        y.append(row[2])

    gr = TGraph( n, x, y )
    gr.SetLineColor( 2 )
    gr.SetLineWidth( 4 )
    gr.SetMarkerColor( 4 )
    gr.SetMarkerStyle( 21 )
    gr.SetTitle( 'Efficiency plan %d' % ich )
    gr.GetXaxis().SetTitle( 'HV' )
    gr.GetYaxis().SetTitle( 'Eff global' )
    gr.Draw( 'AP' )
    c1.Update()
    c1.GetFrame().SetFillColor(0)
    c1.GetFrame().SetBorderSize( 12 )
    c1.Modified()
    c1.SaveAs("plateau%d.png" % ich)
    time.sleep(1)
