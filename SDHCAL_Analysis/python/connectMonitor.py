from ROOT import *
import os
import socket
import httplib, urllib
from copy import deepcopy
from operator import itemgetter
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

class connectMonitor:
    def __init__(self,theurl):
        self.url='lyopc252.in2p3.fr:8000'
    def startFile(self,name):
        
        conn = httplib.HTTPConnection(self.url)
        saction='/startFile?name=%s' % name
        conn.request("GET",saction)
        r1 = conn.getresponse()
        print r1.status, r1.reason
        data1 = r1.read()
        print data1
        conn.close()
    def getHistoList(self):
        conn = httplib.HTTPConnection(self.url)
        saction='/getHistoList'
        conn.request("GET",saction)
        r1 = conn.getresponse()
        data1 = r1.read()
        s=data1.replace('\\n','')
        u=json.loads(s)
        sxml=u.get('getHistoListResponse').get('getHistoListResult')[0]
        tree = ET.XML(sxml)
        apps=tree.findall("Histo")
        flat =[]
        for i in range(len(apps)):
            print apps[i].text
            flat.append(apps[i].text)
        conn.close()
        return flat
    def getHisto(self,name):
        conn = httplib.HTTPConnection(self.url)
        saction='getHisto?name=%s' % name

        conn.request("GET",saction)
        r1 = conn.getresponse()
        print r1.status, r1.reason
        data1 = r1.read()
        s=data1.replace('\\n','')
        u=json.loads(data1)
        sxml=u.get('getHistoResponse').get('getHistoResult')[0]
        tree = ET.XML(sxml)
        h=TBufferXML.ConvertFromXML(ET.tostring(tree))
        conn.close()
        return h
