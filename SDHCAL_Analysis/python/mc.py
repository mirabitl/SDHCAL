import os
import socket
import httplib, urllib
import json
try:
   import elementtree.ElementTree as ET
   from elementtree.ElementTree import parse,fromstring, tostring
   import sqlite as SQLITE
except:
   print "SLC6"
   import xml.etree.ElementTree as ET
   from xml.etree.ElementTree import parse,fromstring, tostring
   import sqlite3 as SQLITE
from copy import deepcopy
from operator import itemgetter
from ROOT import *
import time

from threading import Thread
class threadedSendCommand(Thread):
   """
   Class to paralellize command configuration
   """
   def __init__ (self,dif,command):
      """
      Thread inialisation
      """
      Thread.__init__(self)
      self.dif = dif
      self.status = -1
      self.command=command
   def run(self):
      """
      Thread running
      """
      self.dif.configured=False
      self.dif.failed=False
      try:
         self.dif.sendCommand(self.command)
         self.dif.configured=True
      except:
         self.dif.failed=True


def startFile(host,port,direc):
   lq={}
   lq['name']=direc
   lqs=urllib.urlencode(lq)
   myurl = host+ ":%d" % (port)
   conn = httplib.HTTPConnection(myurl)
   saction = '/startFile?%s' % (lqs)
   conn.request("GET",saction)
   r1 = conn.getresponse()
   print r1.status, r1.reason
         
def startMonitoring(host,port,direc,nd,nr):
   lq={}
   lq['directory']=direc
   lq['ndif']=nd
   lq['run']=nr
   lqs=urllib.urlencode(lq)
   myurl = host+ ":%d" % (port)
   conn = httplib.HTTPConnection(myurl)
   saction = '/startMonitoring?%s' % (lqs)
   conn.request("GET",saction)
   r1 = conn.getresponse()
   print r1.status, r1.reason
def stopMonitoring(host,port):
   myurl = host+ ":%d" % (port)
   conn = httplib.HTTPConnection(myurl)
   saction = '/stopMonitoring'
   conn.request("GET",saction)
   r1 = conn.getresponse()
   print r1.status, r1.reason



def getHistoList(host,port):
  """
  Histogram Monitoring. Send a 'HistoList' request to the application (host:port/theclass,instance) 
  handler is currently implemented in MarlinAnalyzer
  Histo Names are just printed out
  """
  myurl = host+ ":%d" % (port)
  conn = httplib.HTTPConnection(myurl)
  saction = '/getHistoList'
  conn.request("GET",saction)
  r1 = conn.getresponse()
  print r1.status, r1.reason
  data1 = r1.read()
  data=json.loads(data1);
  #print data
  s=data["getHistoListResponse"]["getHistoListResult"][0].replace('\n','')
  #s=data.replace('\n','')
  tree = ET.XML(s)
 
  apps=tree.findall("Histo")
  flat =[]
  for i in range(len(apps)):
    print apps[i].text
    flat.append(apps[i].text)
  conn.close()
  return flat
 
def getHisto(host,port,hname):
  """
  Histogram Monitoring.Send a 'HistoRequest' request to the application (host:port/theclass,instance) for the histo named hname
   handler is currently implemented in MarlinAnalyzer
  It returns the XML version of the object required and book it in pyROOT
  The histo can then be displayed with h.Draw() with all usual ROOT options
  It has to be avoided for 3D or large 2D histograms since the actual size in XML format might be critical
  """
  lq={}
  lq['name']=hname
  lqs=urllib.urlencode(lq)
  myurl = host+ ":%d" % (port)
  conn = httplib.HTTPConnection(myurl)
  saction = '/getHisto?%s' % (lqs)
  conn.request("GET",saction)
  r1 = conn.getresponse()
  print r1.status, r1.reason
  data1 = r1.read()
  data=json.loads(data1);
  #print data
  s=data["getHistoResponse"]["getHistoResult"][0]#.replace('\n','')
  #s=s.replace('\n','gogo')
  #print s
  tree = ET.XML(s)
  #print tree
  h=TBufferXML.ConvertFromXML(ET.tostring(tree))
  return h

def GetEff(host,port,plan):
  l=[]
  dirname='/Plan%d' % plan
  extname= dirname+'/ext'
  nearname= dirname+'/found'
  mulname= dirname+'/mul'
  hext = getHisto(host,port,extname)
  hnear = getHisto(host,port,nearname)
  hmul = getHisto(host,port,mulname)
  hext.Draw("COLZ")
  
  hnear.Draw("COLZ")
  rs=8
  if (hext.GetEntries()<1E6):
    hext.Rebin2D(rs,rs)
    hnear.Rebin2D(rs,rs)
    hmul.Rebin2D(rs,rs)
  heff = hnear.Clone("heff")
  heff.SetDirectory(0)
  heff.Divide(hnear,hext,100.,1.)
  hmulc = hmul.Clone("hmulc")
  hmulc.SetDirectory(0)
  hmulc.Divide(hmul,hnear,1.,1.)
  hmulc.Draw("COLZ")
 
  l.append(hnear)
  l.append(hext)
  l.append(heff)
  heffsum=TH1F("Summary%d" % plan ,"Summary for plan %d " % plan,404,-0.5,100.5)
  hmulsum=TH1F("Summul%d" % plan ,"Multiplicity for plan %d " % plan,200,-0.1,7.1)
  st = ''
  ntk=0;
  for i in range(2,heff.GetXaxis().GetNbins()-1):
    for j in range(2,heff.GetYaxis().GetNbins()-1):
      st = st + '%f ' % heff.GetBinContent(i+1,j+1)
      ntk=ntk+hext.GetBinContent(i+1,j+1)
      if (hext.GetBinContent(i+1,j+1)>5):
        heffsum.Fill(heff.GetBinContent(i+1,j+1))
      hmulsum.Fill(hmulc.GetBinContent(i+1,j+1))
  #print '%s' % st
  l.append(heffsum)
  l.append(hmulc)
  l.append(hmulsum)
  print plan,heffsum.GetMean(),ntk
  return l
