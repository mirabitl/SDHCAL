#!/usr/bin/env python
import os
import socks
import socket
import httplib, urllib,urllib2
from urllib2 import URLError, HTTPError
import json
from copy import deepcopy

import time
import argparse

sockport=None
sp=os.getenv("SOCKPORT","Not Found")
if (sp!="Not Found"):
    sockport=int(sp)


if (sockport !=None):
    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "127.0.0.1", results.sockport)
    socket.socket = socks.socksocket

def parseReturn(command,sr):
 
    if (command=="hvStatus"):
        sj=json.loads(sr)
        ssj=sj["answer"]["HVSTATUS"]
        #print ssj
        print "\033[1m %5s %15s %10s %10s %10s %10s \033[0m" % ('Chan','Name','VSET','ISET','VOUT','IOUT')
        for x in ssj:
            print "#%.4d %15s %10.2f %10.2f %10.2f %10.2f" % (x['channel'],x['name'],x['vset'],x['iset'],x['vout'],x['iout'])
   
        
def executeFSM(host,port,prefix,cmd,params):
   if (params!=None):
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       #if (name!=None):
       #    lq['name']=name
       #if (value!=None):
       #    lq['value']=value
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       lq={}
       
       lq["content"]=json.dumps(params,sort_keys=True)
       #for x,y in params.iteritems():
       #    lq["content"][x]=y
       lq["command"]=cmd           
       lqs=urllib.urlencode(lq)
       #print lqs
       saction = '/%s/FSM?%s' % (prefix,lqs)
       myurl=myurl+saction
       #print myurl
       req=urllib2.Request(myurl)
       r1=urllib2.urlopen(req)
       return r1.read()

def executeCMD(host,port,prefix,cmd,params):
   if (params!=None and cmd!=None):
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       #if (name!=None):
       #    lq['name']=name
       #if (value!=None):
       #    lq['value']=value
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       lq={}
       lq["name"]=cmd
       for x,y in params.iteritems():
           lq[x]=y
       lqs=urllib.urlencode(lq)
       saction = '/%s/CMD?%s' % (prefix,lqs)
       myurl=myurl+saction
       #print myurl
       req=urllib2.Request(myurl)
       try:
           r1=urllib2.urlopen(req)
       except URLError, e:
           p_rep={}
           p_rep["STATE"]="DEAD"
           return json.dumps(p_rep,sort_keys=True)
       else:
           return r1.read()
   else:
       myurl = "http://"+host+ ":%d/%s/" % (port,prefix)
       #conn = httplib.HTTPConnection(myurl)
       #print myurl
       req=urllib2.Request(myurl)
       try:
           r1=urllib2.urlopen(req)
       except URLError, e:
           p_rep={}
           p_rep["STATE"]="DEAD"
           return json.dumps(p_rep,sort_keys=True)
       else:
           return r1.read()
    

