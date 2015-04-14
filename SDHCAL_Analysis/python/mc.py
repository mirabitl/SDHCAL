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

