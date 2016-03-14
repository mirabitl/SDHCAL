#!/usr/bin/env python
import os
import socks
import socket
import httplib, urllib,urllib2
import json
from copy import deepcopy

import time
import argparse


parser = argparse.ArgumentParser()
parser.add_argument('-a', action='store', dest='cmd',default='status',help='action to be done')
parser.add_argument('-c', action='store', dest='config',default=None,help='python config file')
parser.add_argument('--socks', action='store', dest='sockport',default=None,help='use SOCKS port ')
parser.add_argument('--dbstate', action='store', default=None,dest='dbstate',help='set the dbstate')
parser.add_argument('--ctrlreg', action='store', default=None,dest='ctrlreg',help='set the dbstatectrreg in hexa')
parser.add_argument('--version', action='version', version='%(prog)s 1.0')
parser.add_argument('--state', action='store', type=str,default=None,dest='fstate',help='set the rpcdaq state')

results = parser.parse_args()

# Analyse results
if (results.cmd == "forceState" and results.fstate==None):
    print "please specify forceState with --fstate=new_state"
    exit(0)
if (results.config==None):
    print "please specify a configuration with -c conf_name"
    exit(0)

if (results.sockport !=None):
    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "127.0.0.1", results.sockport)
    socket.socket = socks.socksocket
    print "on utilise sock"


# import the configuration
try:
    exec("import %s  as conf" % results.config)
except ImportError:
    raise Exception("cannot import")

# fill parameters 
p_par={}
if (conf.state!=None):
   p_par['dbstate']=conf.state
p_par['zupdevice']=conf.zupdevice
p_par['zupport']=conf.zupport
p_par['writerdir']=conf.directory
p_par['ctrlreg']=conf.register
p_par['dccname']=conf.dccname
p_par['daqhost']=conf.daqhost
p_par['daqport']=conf.daqport
p_par['json']=conf.jsonfile

# specific updates
if (results.cmd != "setParameters" and results.dbstate!=None):
    print "please use --dbstate=new_state with setParameters command"
    exit(0)
if (results.dbstate!=None):
    p_par['dbstate']=results.dbstate

if (results.cmd != "setParameters" and results.ctrlreg!=None):
    print "please use --ctrlreg=new-reg (hexa) with setParameters command"
    exit(0)

if (results.ctrlreg!=None):
    p_par['ctrlreg']=int(results.ctrlreg,16)
    

l_par=json.dumps(p_par,sort_keys=True)


def sendcommand2(command,host=p_par["daqhost"],port=p_par['daqport'],direc=None):
   lq={}
   if (direc!=None):
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       lq['name']=direc
       lqs=urllib.urlencode(lq)
       saction = '/%s?%s' % (command,lqs)
       myurl=myurl+saction
       print myurl
       req=urllib2.Request(myurl)
       r1=urllib2.urlopen(req)

       return r1.read()
   else:
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       saction = '/%s' % command
       myurl=myurl+saction
       print myurl
       req=urllib2.Request(myurl)
       r1=urllib2.urlopen(req)
       if (command=="status"):
           s=r1.read()
           sj=json.loads(s)
           ssj=json.loads(sj["statusResponse"]["statusResult"][0])
#           for x in ssj:
#             for d in x["difs"]:
#                print '#%4d %5x %6d %12d %12d %s %s ' % (d["id"],d["slc"],d["gtc"],d["bcid"],d["bytes"],d["state"],x["name"])
           print "\033[1m %4s %5s %6s %12s %12s %15s  %s \033[0m" % ('DIF','SLC','EVENT','BCID','BYTES','SERVER','STATUS')

           for x in ssj:
              for d in x["difs"]:
                  print '#%4d %5x %6d %12d %12d %15s %s ' % (d["id"],d["slc"],d["gtc"],d["bcid"],d["bytes"],x["name"],d["state"])
       elif (command=="jobStatus"):
           s=r1.read()
           sj=json.loads(s)
           ssj=json.loads(sj["jobStatusResponse"]["jobStatusResult"][0])
           print "\033[1m %6s %15s %25s %20s \033[0m" % ('PID','NAME','HOST','STATUS')
           for x in ssj:
               if (x['DAQ']=='Y'):
                   print "%6d %15s %25s %20s" % (x['PID'],x['NAME'],x['HOST'],x['STATUS'])
       else:
          print r1.read()
          return r1.read()

       
       #print r1.status, r1.reason


def sendcommand(command,host=p_par["daqhost"],port=p_par['daqport'],direc=None):
   lq={}
   if (direc!=None):
      lq['name']=direc
      lqs=urllib.urlencode(lq)
      myurl = host+ ":%d" % (port)
      conn = httplib.HTTPConnection(myurl)
      saction = '/%s?%s' % (command,lqs)
      #print saction
      conn.request("GET",saction)
      r1 = conn.getresponse()
      return r1.read()
      print r1.status, r1.reason
   else:
       myurl = host+ ":%d" % (port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/%s' % command
       conn.request("GET",saction)
       r1 = conn.getresponse()
       if (command!="status"):
          print r1.read()
          return r1.read()
       else:
          s=r1.read()
          sj=json.loads(s)
          ssj=json.loads(sj["statusResponse"]["statusResult"][0])
          for x in ssj:
              print "\033[1m %4s %5s %6s %12s %12s %20s  %s \033[0m" % ('DIF','SLC','EVENT','BCID','SERVER','STATUS')
              for d in x["difs"]:
                  print '#%4d %5x %6d %12d %12d %s %s ' % (d["id"],d["slc"],d["gtc"],d["bcid"],d["bytes"],d["name"],x["state"])

       
          #print r1.status, r1.reason

if (results.cmd == "forceState"):
    sendcommand2(results.cmd,direc=results.fstate);
    exit(0)
if (results.cmd == "setParameters"):
    print "Setting ",l_par
    print sendcommand2(results.cmd,direc=l_par)
    exit(0)
if (results.cmd == "createJobControl"):
    print sendcommand2(results.cmd,direc=p_par["json"])
    exit(0)
else:
    sendcommand2(results.cmd)
"""
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
"""
