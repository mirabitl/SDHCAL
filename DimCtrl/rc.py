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

help_action="Action to be execute \n"
help_action=help_action+" \n \t createDaq : create the RpcDaq object to communicate with the DAQ \n"
help_action=help_action+" \n \t createJobControl : create the DimJobControlInterface object to communicate with the job control \n"

help_action=help_action+" \n \t createSlowControl : create the DimSlowControl object to communicate with the wiener and BMP183 servers \n"

help_action=help_action+" \n \t Discover : create the RpcDaq object to communicate with the DAQ \n"

def dohelp():
    return help_action
parser.add_argument('-a', action='store', dest='cmd',choices=('createDaq','createJobControl','createSlowControl','Discover','setParameters','initialise','configure','start','status','shmStatus','stop','destroy','jobStatus','jobStartAll','jobKillAll','downloadDB','setControlRegister','hvStatus','PT','setVoltage','setCurrentLimit','HVON','HVOFF','initialiseDB','startStorage','stopStorage','starCheck','stopCheck','loadReferences'),
                    default='jobStatus',help='Action to be done choices, {%(choices)s ')
parser.add_argument('-c', action='store', dest='config',default=None,help='python config file')
parser.add_argument('--socks', action='store', type=int,dest='sockport',default=None,help='use SOCKS port ')
parser.add_argument('--dbstate', action='store', default=None,dest='dbstate',help='set the dbstate')
parser.add_argument('--ctrlreg', action='store', default=None,dest='ctrlreg',help='set the dbstatectrreg in hexa')
parser.add_argument('--version', action='version', version='%(prog)s 1.0')
parser.add_argument('--state', action='store', type=str,default=None,dest='fstate',help='set the rpcdaq state')
parser.add_argument('--channel', action='store',type=int, default=None,dest='channel',help='set the hvchannel')
parser.add_argument('--voltage', action='store',type=float, default=None,dest='voltage',help='set the hv voltage')
parser.add_argument('--current', action='store',type=float, default=None,dest='current',help='set the hv current')

parser.add_argument('--period', action='store',type=int, default=None,dest='period',help='set the tempo period')

parser.add_argument('--account', action='store', default=None,dest='account',help='set the mysql account')

results = parser.parse_args()

# Analyse results
if (results.config==None):
    dc=os.getenv("DAQCONFIG","Not Found")
    if (dc=="Not Found"):
        print "please specify a configuration with -c conf_name"
        exit(0)
    else:
        results.config=dc

if (results.sockport !=None):
    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "127.0.0.1", results.sockport)
    socket.socket = socks.socksocket
    print "on utilise sock",results.sockport


# import the configuration
try:
    exec("import %s  as conf" % results.config)
except ImportError:
    raise Exception("cannot import")

if (results.cmd==None):
    raise Exception("Please specify a command")

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

l_par=json.dumps(p_par,sort_keys=True)

lcgi={}
v_name=None
v_value=None
# specific updates
if (results.fstate !=None):
    results.cmd="forceState"
    v_name=results.fstate
    lcgi['name']=v_name
if (results.dbstate !=None):
    lcgi['name']=v_name
if (results.ctrlreg !=None):
    lcgi['value']=v_value
if (results.cmd =="setParameters"):
    v_name=l_par
    lcgi['name']=v_name
if (results.cmd =="createJobControl"):
    v_name=conf.jsonfile
    lcgi['name']=v_name
if (results.channel !=None):
    lcgi['channel']=results.channel
if (results.voltage !=None):
    lcgi['V']=results.voltage
if (results.current !=None):
    lcgi['I']=results.current
if (results.period !=None):
    lcgi['period']=results.period
if (results.account !=None):
    lcgi['account']=results.account

    
print lcgi
#exit(0)

def sendcommand2(command,host=p_par["daqhost"],port=p_par['daqport'],lq=None):
   
   if (lq!=None):
       if (len(lq)!=0):
           myurl = "http://"+host+ ":%d" % (port)
           #conn = httplib.HTTPConnection(myurl)
           #if (name!=None):
           #    lq['name']=name
           #if (value!=None):
           #    lq['value']=value
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
       elif (command=="hvStatus"):
           s=r1.read()
           sj=json.loads(s)
           ssj=json.loads(sj["hvStatusResponse"]["hvStatusResult"][0])
           print "\033[1m %5s %10s %10s %10s %10s \033[0m" % ('Chan','VSET','ISET','VOUT','IOUT')
           for x in ssj:
               print "#%.4d %10.2f %10.2f %10.2f %10.2f" % (x['channel'],x['vset'],x['iset'],x['vout'],x['iout'])

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
if (len(lcgi)!=0):
    sendcommand2(results.cmd,lq=lcgi)
else:
    sendcommand2(results.cmd)
exit(0)
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
