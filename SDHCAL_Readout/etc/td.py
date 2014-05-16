import os
import socket
import httplib, urllib
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

import postelog

def cleanrpi(host):
   os.system("ssh pi@"+host+" '/opt/dhcal/bin/ListDevices.py'")
   os.system("ssh pi@"+host+" 'sudo /etc/init.d/difd stop'")
   os.system("ssh pi@"+host+" 'sudo /etc/init.d/difd start'")
   os.system("ssh pi@"+host+" 'sudo /etc/init.d/difd status'")
   os.system("ssh pi@"+host+" 'sudo /etc/init.d/dccd stop'")
   os.system("ssh pi@"+host+" 'sudo /etc/init.d/dccd start'")
   os.system("ssh pi@"+host+" 'sudo /etc/init.d/dccd status'")


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
         



def parseConfigurationFile(fname):
  """
  Parsing of a configuration file
  """
  urllist =[]
  StandaloneManagerUrl=None
  tree2=ET.parse(fname)
  for node in tree2.getiterator():
    #print node.tag
    ff=node.tag.split('}')
    if (len(ff)<2): continue;
    prefix=ff[0]+'}'
    name=ff[1]
    if (name == "Context"):
      for x in node.items():
        if (x[0] == "url"):
          urllist.append(x[1])
          currentContext=x[1]
    if (name == "Application"):
      print node.items()
      for x in node.items():
        if (x[0] == "class"):
          currentApp=x[1]
          if (currentApp=="StandaloneManager"):
            StandaloneManagerUrl=currentContext
  print " List of URLs",urllist
  print "Local Manager is ",StandaloneManagerUrl


def sendConfigurationKillCommand(theurl,jobId):
  """
  Send a SOAP command to the job control to kill a xdaq process:
  theurl = URL of the job control
  jobid = job id of the xdaq process
  """

  ff=theurl.split(':')
  contextPort=ff[2]
  host=ff[1][2:len(ff[1])]
  thelid = 10
  port=9999
  msg = '<?xml version="1.0" encoding="UTF-8"?>  \n'
  msg = msg+ '<SOAP-ENV:Envelope '
  msg = msg+ ' SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"'
  msg = msg+ ' xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"'
  msg = msg+ ' xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
  msg = msg+ ' xmlns:xsd="http://www.w3.org/2001/XMLSchema"'
  msg = msg+ ' xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"'
  msg = msg+ '>\n'
  msg = msg+   '<SOAP-ENV:Header>\n'
  msg = msg+   '</SOAP-ENV:Header>\n'
  msg = msg+   '<SOAP-ENV:Body>\n'
#  msg = msg+     '<xdaq:startXdaqExe  xmlns:xdaq="urn:xdaq-soap:3.0" execPath="/opt/xdaq/bin/xdaq.exe" user="acqilc" >\n'
  msg = msg+     '<xdaq:killExec user="acqilc" jid="'+jobId+'" xmlns:xdaq="urn:xdaq-soap:3.0" />\n'
  msg = msg+   '</SOAP-ENV:Body>\n'
  msg = msg+ '</SOAP-ENV:Envelope>\n'
  smsg0=msg.encode("utf_8")
  fout=open("toto.txt","w+")
  fout.write(smsg0)
  tree = ET.XML(msg)
  smsg=tostring(tree)
  sm1=smsg.replace('ns0','SOAP-ENV')
  sm2=sm1.replace('ns1','xdaq')
  sm3=sm2.replace('ns2','xc')
  sm4=sm3.replace('ns3','xc')
  print sm4
  print  "======================================================================" 
  saction = 'urn:xdaq-application:lid=%d' % (thelid)
  #params = urllib.urlencode(msg)
  headers = {"Content-Type":"text/xml", "charset":"utf-8","Content-Description":"SOAP Message", "SOAPAction":saction}
  myurl = host+ ":%d" % port

  conn = httplib.HTTPConnection(myurl)
  myreq=conn.request("POST", "/cgi-bin/query", smsg, headers)
  response = conn.getresponse()
  if (response.status != 200):
    print response.status, response.reason
  data = response.read()
  print response.status
  
  conn.close()
  return data


def sendConfigurationStartCommand(theurl,fName,MARLINWORKDIR="/data/online/MARLIN/MARLIN_DHCALOnline",MARLINLIB="libDHCALOnline.so"):
  """
  Send a SOAP command to the job contrrol to start a XDAQ process:
  theurl = URL of the job control
  fName = File name containing the XML configuration
  MARLINWORKDIR = Marlin processor directory
  MARLINDLL = share librariry name
  BE CAREFULL some configuration are hardocded in this function and will have to be modified:

  MARLINDLL and MARLINWORKDIR points to the standard beam test analysos
  
  """

  ff=theurl.split(':')
  contextPort=ff[2]
  host=ff[1][2:len(ff[1])]
  thelid = 10
  port=9999
  msg = '<?xml version="1.0" encoding="UTF-8"?>  \n'
  msg = msg+ '<SOAP-ENV:Envelope '
  msg = msg+ ' SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"'
  msg = msg+ ' xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"'
  msg = msg+ ' xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
  msg = msg+ ' xmlns:xsd="http://www.w3.org/2001/XMLSchema"'
  msg = msg+ ' xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"'
  msg = msg+ '>\n'
  msg = msg+   '<SOAP-ENV:Header>\n'
  msg = msg+   '</SOAP-ENV:Header>\n'
  msg = msg+   '<SOAP-ENV:Body>\n'
#  msg = msg+     '<xdaq:startXdaqExe  xmlns:xdaq="urn:xdaq-soap:3.0" execPath="/opt/xdaq/bin/xdaq.exe" user="acqilc" >\n'
  msg = msg+     '<xdaq:startXdaqExe execPath="/opt/xdaq/bin/xdaq.exe" user="acqilc" argv="-p '+contextPort+' -l ERROR" xmlns:xdaq="urn:xdaq-soap:3.0" >\n'
  #msg = msg+     '<xdaq:startXdaqExe execPath="/data/online/utils/privilegeXdaq" user="acqilc" argv="-p '+contextPort+' -l ERROR" xmlns:xdaq="urn:xdaq-soap:3.0" >\n'
  dimnode=os.getenv('DIM_DNS_NODE')
  if (dimnode == None):
    dimnode=socket.gethostname()
  confdb=os.getenv('CONFDB')
  
  marlinlibpath=MARLINWORKDIR+"/lib"
  marlindll=marlinlibpath+"/"+MARLINLIB
  msg= msg+ '<EnvironmentVariable XDAQ_ROOT="/opt/xdaq" XDAQ_OS="linux"  XDAQ_PLATFORM="x86_slc4" XDAQ_DOCUMENT_ROOT="/opt/xdaq/htdocs" XDAQ_ELOG="SET" DIM_DNS_NODE="'+dimnode+'" ROOTSYS="/data/ilcsoft/root/root" CONFDB="'+confdb+'" LCIODIR="/data/online/lcio/v01-60/" LD_LIBRARY_PATH="/opt/dhcal/lib:/data/online/opt/dhcal/lib:'+marlinlibpath+':/usr/local/root_v32/lib:/usr/local/lib:/opt/xdaq/lib:/data/online/opt/dhcal/lib:/opt/dhcal/lib:/data/online/lcio/v01-60/lib:/usr/local/root_v32/lib" >\n'
  msg = msg+   '</EnvironmentVariable>\n'
  msg= msg+ '<EnvironmentVariable MARLIN_DLL="'+marlindll+'" MARLINWORKDIR="'+MARLINWORKDIR+'" >\n'
  msg = msg+   '</EnvironmentVariable>\n'
  msg = msg+     '<ConfigFile>\n'
  msg = msg+ '<![CDATA[\n'
  
  ftmp=open(fName)
  flines = ftmp.readlines()
  for l in flines:
    msg = msg + l
  
  msg = msg+ ']]>\n'
  msg = msg+     '</ConfigFile>\n'
  msg = msg+     '</xdaq:startXdaqExe>\n'
  msg = msg+   '</SOAP-ENV:Body>\n'
  msg = msg+ '</SOAP-ENV:Envelope>\n'
  smsg0=msg.encode("utf_8")
  fout=open("toto.txt","w+")
  fout.write(smsg0)
  tree = ET.XML(msg)
  smsg=tostring(tree)
  sm1=smsg.replace('ns0','SOAP-ENV')
  sm2=sm1.replace('ns1','xdaq')
  sm3=sm2.replace('ns2','xc')
  sm4=sm3.replace('ns3','xc')
  #print sm4
  #print  "======================================================================" 
  saction = 'urn:xdaq-application:lid=%d' % (thelid)
  #params = urllib.urlencode(msg)
  headers = {"Content-Type":"text/xml", "charset":"utf-8","Content-Description":"SOAP Message", "SOAPAction":saction}
  myurl = host+ ":%d" % port

  conn = httplib.HTTPConnection(myurl)
  myreq=conn.request("POST", "/cgi-bin/query", smsg, headers)
  response = conn.getresponse()
  if (response.status != 200):
    print response.status, response.reason
  data = response.read()
  print response.status
  
  conn.close()
  return data


def sendSOAPCommand(host,port,theclass,instance,cmdName):
  """
  Send a SOAP command to:
  host = target host
  port = target port
  theclass = class Name
  instance = instance number
  cmdName = command name
  """
  msg = ""
  msg = msg+ '<SOAP-ENV:Envelope'
  msg = msg+ ' SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"'
  msg = msg+ ' xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"'
  msg = msg+ ' xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
  msg = msg+ ' xmlns:xsd="http://www.w3.org/2001/XMLSchema"'
  msg = msg+ ' xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"'
  msg = msg+ '>'
  msg = msg+   '<SOAP-ENV:Header>'
  msg = msg+   '</SOAP-ENV:Header>'
  msg = msg+   '<SOAP-ENV:Body>'
  msg = msg+     '<xdaq:' +cmdName+' xmlns:xdaq="urn:xdaq-soap:3.0"/>'
  msg = msg+   '</SOAP-ENV:Body>'
  msg = msg+ '</SOAP-ENV:Envelope>'
#  print msg
  saction = 'urn:xdaq-application:class=%s,instance=%d' % (theclass,instance)
  #params = urllib.urlencode(msg)
  headers = {"Content-Type":"text/xml", "Content-Description":"SOAP Message", "SOAPAction":saction}
  myurl = host+ ":%d" % port

  conn = httplib.HTTPConnection(myurl)
  myreq=conn.request("POST", "/cgi-bin/query", msg, headers)
  response = conn.getresponse()
  if (response.status != 200):
    print response.status, response.reason
  data = response.read()
  #print data
  
  conn.close()
  return data

def parseQuery(theclass,data):
  """
  XML parsing of Parameteruery command
  it retrieves all parameters of the classe defined by theclass name
  and returns a dictionnary of the parameters conataining  a list of 2 strings:  
  dict['paramName'][0] = value
  dcit['paramName'][1] = type
  """
  tree = ET.XML(data)
  prefix = "{urn:xdaq-application:%s}" % theclass
  l ={}
  for node in tree.getiterator():
    if (node.text!=None):
      s=[]
      s.append(node.text)
      s.append(node.attrib['{http://www.w3.org/2001/XMLSchema-instance}type'])
      l[node.tag.replace(prefix,'')]=s
  return l

def getAppList(host,port,theclass,instance):
  """
  It gets the list of all XDAQ 
  applications declared by sending an 'appallrequest' to the application
  theclass , instance on host:port
  The handler of such command is instantiate in the JsInterface provided 
  with the XDAQ SDHCAL software. Any application inheriting from JsInterface
  will reply
  """
  myurl = host+ ":%d" % (port)
  conn = httplib.HTTPConnection(myurl)
  saction = '/urn:xdaq-application:class=%s,instance=%d/appallrequest' % (theclass,instance)
  conn.request("GET",saction)
  r1 = conn.getresponse()
  print r1.status, r1.reason
  data1 = r1.read()
    #print data1
  s=data1.replace('\n','')
  tree = ET.XML(s)
  apps=tree.findall("XdaqApplication")
  flat =[]
  for i in range(len(apps)):
    s ={}
    s['Class']=apps[i].findtext("Class")
    s['Url']=apps[i].findtext("Url")
    s['Urn']=apps[i].findtext("Urn")
    s['Instance']=apps[i].findtext("Instance")
    flat.append(s)

    appmap={}
    for x in flat:
      if (not( x['Class'] in appmap)):
        appmap[x['Class']]=[]
    for x in flat:
      appmap[x['Class']].append(x)

  conn.close()
  return appmap


def getHistoList(host,port,theclass,instance):
  """
  Histogram Monitoring. Send a 'HistoList' request to the application (host:port/theclass,instance) 
  handler is currently implemented in MarlinAnalyzer
  Histo Names are just printed out
  """
  myurl = host+ ":%d" % (port)
  conn = httplib.HTTPConnection(myurl)
  saction = '/urn:xdaq-application:class=%s,instance=%d/HistoList' % (theclass,instance)
  conn.request("GET",saction)
  r1 = conn.getresponse()
  print r1.status, r1.reason
  data1 = r1.read()
  print data1
  s=data1.replace('\n','')
  tree = ET.XML(s)
  apps=tree.findall("Histo")
  flat =[]
  for i in range(len(apps)):
    print apps[i].text
    flat.append(apps[i].text)
  conn.close()
  return flat
def getHisto(host,port,theclass,instance,hname):
  """
  Histogram Monitoring.Send a 'HistoRequest' request to the application (host:port/theclass,instance) for the histo named hname
   handler is currently implemented in MarlinAnalyzer
  It returns the XML version of the object required and book it in pyROOT
  The histo can then be displayed with h.Draw() with all usual ROOT options
  It has to be avoided for 3D or large 2D histograms since the actual size in XML format might be critical
  """
  lq={}
  lq['Histo']=hname
  lqs=urllib.urlencode(lq)
  myurl = host+ ":%d" % (port)
  conn = httplib.HTTPConnection(myurl)
  saction = '/urn:xdaq-application:class=%s,instance=%d/HistoRequest?%s' % (theclass,instance,lqs)
  conn.request("GET",saction)
  r1 = conn.getresponse()
  print r1.status, r1.reason
  data1 = r1.read()
  s=data1.replace('\n','')
  tree = ET.XML(s)
  h=TBufferXML.ConvertFromXML(ET.tostring(tree))
  return h

def getParameterList(host,port,theclass,instance):
  """
  Standard XDAQ ParameterQuery on (host:port/theclass,instance) application
  It returns the XML string (ParseQuery call is needed to get comprehensive set of parameters)
  """
  myurl = host+ ":%d" % (port)
  conn = httplib.HTTPConnection(myurl)
  saction = '/urn:xdaq-application:class=%s,instance=%d/request' % (theclass,instance)
  conn.request("GET",saction)
  r1 = conn.getresponse()
  print r1.status, r1.reason
  data1 = r1.read()
  print data1
  conn.close()
  return data1


def setParameter(host,port,theclass,instance,paramName,paramType,paramValue):
  """
  Standard XDAQ ParameterSet on (host:port/theclass,instance) application
  parameName,paramType, and paramValue are strings
  Usually they are extracted from the dictionnary created by parseQuery
  """
  msg = ""
  msg = msg+  '<SOAP-ENV:Envelope'
  msg = msg+  ' SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"'
  msg = msg+  ' xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"'
  msg = msg+  ' xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
  msg = msg+  ' xmlns:xsd="http://www.w3.org/2001/XMLSchema"'
  msg = msg+  ' xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"'
  msg = msg+  '>'
  msg = msg+    '<SOAP-ENV:Header>'
  msg = msg+    '</SOAP-ENV:Header>'
  msg = msg+    '<SOAP-ENV:Body>'
  msg = msg+      '<xdaq:ParameterSet'
  msg = msg+      ' xmlns:xdaq="urn:xdaq-soap:3.0"'
  msg = msg+      '>'	
  msg = msg+        '<p:properties'
  msg = msg+        ' xmlns:p="urn:xdaq-application:%s"' % theclass
  msg = msg+        ' xsi:type="soapenc:Struct"'
  msg = msg+        '>'
  msg = msg+          '<p:%s' % paramName
  msg = msg+          ' xsi:type="%s"' % paramType
  msg = msg+          '>'
  msg = msg+            '%s' % paramValue
  msg = msg+          '</p:%s>' % paramName
  msg = msg+        '</p:properties>'
  msg = msg+      '</xdaq:ParameterSet>'
  msg = msg+    '</SOAP-ENV:Body>'
  msg = msg+  '</SOAP-ENV:Envelope>'
  print msg
  saction = 'urn:xdaq-application:class=%s,instance=%d' % (theclass,instance)
  #params = urllib.urlencode(msg)
  headers = {"Content-Type":"text/xml", "Content-Description":"SOAP Message", "SOAPAction":saction}
  myurl = host+ ":%d" % port

  conn = httplib.HTTPConnection(myurl)
  conn.request("POST", "/cgi-bin/query", msg, headers)
  response = conn.getresponse()
  print response.status, response.reason
  data = response.read()
  print data
  print "===================================================="
  conn.close()

class XdaqApplication:
  """
  Handle all application definition and parameters
  """
  def Dump(self):
    """
    Print parameters info
    """
    print self.host_,self.port_,self.className_,self.instance_
    for pname,vpar in sorted(self.params_.iteritems()):
      print "params_[",pname,"]=",vpar[0]

  def sendCommand(self,cmd):
    """
    Send the SOAP command cmd to the application
    """
    a=sendSOAPCommand(self.host_,self.port_,self.className_,self.instance_,cmd)
  def setParameter(self,pname,pval):
    """
    Change internal parameter pname to pval value (both are strings)
    """
    self.params_[pname][0]=pval

  def getParameter(self, pname):
    """
    get the stored value of the parameter pname
    """
    return self.params_[pname][0]

  def QueryParameters(self):
    """
    Query all parameters of the application
    """
    a=sendSOAPCommand(self.host_,self.port_,self.className_,self.instance_,'ParameterQuery')
    self.lastQueryParams_=parseQuery(self.className_,a)
    self.params_ = deepcopy(self.lastQueryParams_)
  def updateParameters(self):
    """
    Update parameters modified since last query, a new query is done 
    """
    updated=False
    for pname,vpar in self.params_.iteritems():
      print self.lastQueryParams_[pname][0],vpar[0]
      if (self.lastQueryParams_[pname][0] != vpar[0]):
        setParameter(self.host_,self.port_,self.className_,self.instance_,pname,vpar[1],vpar[0])
        updated=True
        if (updated):
          print "On update les parametres"
      self.QueryParameters()
        
  def __init__(self,theurl,theurn,theclass,theinstance=None):
    """
    Constructor
    
    """
    self.url_=theurl
    ff=theurl.split(':')
    self.port_=int(ff[2])
    self.host_=ff[1][2:len(ff[1])]
    self.urn_=theurn
    self.className_=theclass
    self.instance_=-1
    if (theinstance!=None):
      self.instance_=int(theinstance)
    self.lastQueryParams_={}
    self.params_ = deepcopy(self.lastQueryParams_)
    self.configured=False
    self.failed=False
class XdaqSetup:
  """
  Access to all apps of a Setup
  """
  def __init__(self,thehost,theport,theapp,theinstance):
    """
    Define the connection to host capable to return app list
     -thehost = hostname
     -theport = port
     -theapp  = APplication Name
     - theinstance  = Instance Number
    """
    self.host_ = thehost
    self.port_= theport
    self.className_ = theapp
    self.instance_ = theinstance
    self.appDesc_={}
    self.xdaqApps_={}


  def QueryApplication(self):
    """
    Get the application list and store them in self.xdaqApps_
    """
    try:
      self.appDesc_ = getAppList(self.host_,self.port_,self.className_,self.instance_)
    except:
      raise NameError('No answer to AppList from %s %d %s %d' % (self.host_,self.port_,self.className_,self.instance_))
    for xgroup,vappdesc in self.appDesc_.iteritems():
      # On cree le vecteur d'application
      self.xdaqApps_[xgroup]=[]
      for apd in vappdesc:
        xda = XdaqApplication(apd['Url'],apd['Urn'],apd['Class'],apd['Instance'])
        if (xda.instance_>=0):
          print apd['Url'],apd['Urn'],apd['Class'],apd['Instance']
          xda.QueryParameters()
        self.xdaqApps_[xgroup].append(xda)


class IlcSetup(XdaqSetup):
  """
  Specialised XDAQ setup.
  It can parse a configuration file and create the corressponding executives.
  It connects to the StandaloneManager instance 0 to get the address of all application
  It has references to:
  The Manager() = StandaloneManager XdaqApplication
  The Analyzer() = MarlinAnalyzer or LCIOAnalyzer XdaqApplication
  The EVM() = rubuilder::evm::Application XdaqApplication

  all others applications are accessible thru xdaqApps_ dictionary
  """
#  def __init__(self,thehost,theport):
#    XdaqSetup.__init__(self,thehost,theport,"StandaloneManager",0)
#    self.initParams()


  def __init__(self,theFileName):
    """
    
     Parse the configuration file:
     theFileName = the XDAQ XML configuration
    """
    if (theFileName!=""):
      print "Parsing the Configuration file"
      self.ConfigurationFile_=theFileName
      self.parseConfigurationFile(self.ConfigurationFile_)
      
  def connect(self,thehost,theport,theApp="StandaloneManager"):
    """
    connect to the StandaloneManager and get the list of applications:
    thehost = Host name of the StandaloneManager
    theport = host port of the StandaloneManager
    """
    XdaqSetup.__init__(self,thehost,theport,theApp,0)
    self.initParams()   

  def CreateExecutives(self,MARLINWORKDIR="/data/online/MARLIN/MARLIN_DHCALOnline",MARLINLIB="libDHCALOnline.so"):
    """
    From the urllist got after parsing of the XDAQ XML configuration:

    - It send a start command for all executives to the jobcontrol application on the corresponding machines
    - it keeps a list of jobs created
    - it connects to the StandaloneManager and get the applications list
    - it stores the daq status and ids in the sqlite DB if connected to it

    if MARLINWORKDIR and library are different from the standard one they can be specified
    MARLINWORKDIR= Processor directory
    MARLINDLL = library name
    
    """
#    self.generator.OFF()
    self.jobList_ ={}
    host=""
    port=0
    for x in self.urllist_:
      rep =sendConfigurationStartCommand(x,self.ConfigurationFile_,MARLINWORKDIR,MARLINLIB)
      print rep
      if (rep == None): return;
      if (x == self.StandaloneManagerUrl_):
        ff=x.split(':')
        port=int(ff[2])
        host=ff[1][2:len(ff[1])]

      tree2=ET.XML(rep)
      for node in tree2.getiterator():
    #print node.tag
        ff=node.tag.split('}')
        if (len(ff)<2): continue;
        prefix=ff[0]+'}'
        name=ff[1]
        if (name == "jid"):
          self.jobList_[x]=node.text
          print "JOB ",x,node.text
    
    if (port>0):
      time.sleep(2)
      print host,port
      XdaqSetup.__init__(self,host,port,"StandaloneManager",0)
      self.initParams()
    print self.jobList_



  def DestroyExecutives(self):
    """
    It destroys executives created in the CreateExecutives call
    """


    for x,y in self.jobList_.iteritems():
      sendConfigurationKillCommand(x,y)
    
  def parseConfigurationFile(self,fname):
    """
    It parse the XDAQ XML configuration, get a list of url and the url of the StandaloneManager
    fname = Configuration file
    """
    self.urllist_ =[]
    StandaloneManagerUrl=None
    tree2=ET.parse(fname)
    for node in tree2.getiterator():
    #print node.tag
      ff=node.tag.split('}')
      if (len(ff)<2): continue;
      prefix=ff[0]+'}'
      name=ff[1]
      if (name == "Context"):
        for x in node.items():
          if (x[0] == "url"):
            self.urllist_.append(x[1])
            currentContext=x[1]
      if (name == "Application"):
        print node.items()
        for x in node.items():
          if (x[0] == "class"):
            currentApp=x[1]
            if (currentApp=="StandaloneManager"):
              self.StandaloneManagerUrl_=currentContext
    print self.urllist_
    print self.StandaloneManagerUrl_


    
  def initParams(self):
    """
    Query of all applications parameters
    """
    self.QueryApplication()
    self.mgr_ = self.xdaqApps_['StandaloneManager'][0]
    if ( 'MarlinAnalyzer' in self.xdaqApps_):
      self.anlr_ =self.xdaqApps_['MarlinAnalyzer'][0]
    else:
      if ( 'LCIOAnalyzer' in self.xdaqApps_):
        self.anlr_ =self.xdaqApps_['LCIOAnalyzer'][0]
      else:
        self.anlr_=None
    if ( 'rubuilder::evm::Application' in self.xdaqApps_):
      self.evm_ = self.xdaqApps_['rubuilder::evm::Application'][0]
    else:
      self.evm_=None
  
  def updateView(self):
    """
    Query parameters of all setup applications 
    """
    for xgroup,vappdesc in self.xdaqApps_.iteritems():
      for apd in vappdesc:
        if (apd.instance_>=0):
          apd.QueryParameters()


  def Manager(self):
    """
    reference to StandaloneManager application
    """
    return self.mgr_
  def Analyzer(self):
    """
    reference to marlinAnalyzer or LCIOAnalyzer  application
    """
    return self.anlr_
  def EVM(self):
    """
    refernce to EVM application
    """
    return self.evm_

  def Initialise(self):
    """
    Local Manager Initialise setup and refresh parameters
    It configures and enables the Event Builder
    """
    self.mgr_.sendCommand("Initialise")
    self.updateView()

  def Configure(self):
    """
    Local Manager Configure setup and refresh parameters
    It configures the ILC applications
    """


    self.mgr_.sendCommand("Configure") 
    self.updateView()
  def StartMonitoring(self):
    """
    Local Manager Configure setup and refresh parameters
    It configures the ILC applications
    """


    self.anlr_.sendCommand("Configure") 
    self.updateView()

  def StopMonitoring(self):
    """
    Local Manager Configure setup and refresh parameters
    It configures the ILC applications
    """


    self.anlr_.sendCommand("Halt") 
    self.updateView()
  def Enable(self):
    """
    Local Manager Enable setup and refresh parameters
    It Enables ILC applications
    """

    self.mgr_.sendCommand("Enable") 
    self.updateView()

  def Stop(self):
    """
    Local Manager Enable setup and refresh parameters
    It Enables ILC applications
    """

    self.mgr_.sendCommand("Stop") 
    self.updateView()

  def Halt(self):
    """
    Local manager Halt setup and refresh parameters
    It halts ILC application
    """
    self.mgr_.sendCommand("Halt") 
    self.updateView()

  def Destroy(self):
    """
    Local manager Halt setup and refresh parameters
    It halts ILC application
    """
    self.mgr_.sendCommand("Destroy") 
    self.updateView()



  def HistoList(self):
    """
    print histgram list from the analyzer
    """
    return getHistoList(self.anlr_.host_,self.anlr_.port_,self.anlr_.className_,self.anlr_.instance_)

  def GetHisto(self,hname):
    """
    return root Histo with name hname
    """
    return getHisto(self.anlr_.host_,self.anlr_.port_,self.anlr_.className_,self.anlr_.instance_,hname)


  def setTriggerMode(self,powerpulsing=True,ilc=True,temperature=True,analog=False,digital=True):
     """
//       0x895A0040 2304376896 SDCC, BT, PP et bits 8-12 =0 dans ctrl reg  temperature digital		OK
//       0x89180000 2300051456 SDCC, ILC PP et bits 8-12 =0 dans ctrl reg  												OK
// 	 0x89580000 2304245760 SDCC ILC PP pwn on lecture digitale																OK
// 	 0x895A0000 2304376832 SDCC ILC PP pwn on lecture digitale	et temperatures 							OK
// 	 0x81981B40 2174229312 SDCC BT  lecture analogique	                         			 				OK
// 	 0x819A1B40 2174360384 SDCC BT  lecture analogique	temperature               						OK
// 			 0x81DA1B40 2178554688 SDCC BT  lecture digitale et analogique	temperature  	 						OK
// 			 0x815A1B00 2170166016 SDCC ILC  lecture digitale	et temperatures 							
// 			 0x815A1B40 2170166080 SDCC BT  lecture digitale	et temperatures 	
     Set the tigger Mode : 
        self explicit        


     """
     #  BT, PP et bits 8-12 =0 dans ctrl reg  temperature digital	
     if ( (not ilc) and powerpulsing and temperature and (not analog) and digital):
        mode= 0x895A0040
     # ILC PP et bits 8-12 =0 dans ctrl reg 
     if ( ilc  and powerpulsing and (not temperature) and (not analog) and digital):
        mode= 0x89580000
     #  ILC PP pwn on lecture digitale	et temperatures
     if ( ilc  and powerpulsing and (temperature) and (not analog) and digital):
        mode= 0x895A0000
     # BT  lecture analogique
     if ( (not ilc)  and (not powerpulsing) and (not temperature) and (analog) and (not digital)):
        mode= 0x81981B40
     # BT  lecture analogique	temperature
     if ( (not ilc)  and (not powerpulsing) and (temperature) and (analog) and (not digital)):
        mode=0x819A1B40
     # BT  lecture digitale et analogique	temperature
     if ( (not ilc)  and (not powerpulsing) and (temperature) and (analog) and (digital)):
        mode=0x81DA1B40
     # ILC  lecture digitale	et temperatures 
     if ( (ilc)  and (not powerpulsing) and (temperature) and (not analog) and (digital)):
        mode=0x815A1B00
     # BT  lecture digitale	et temperatures
     if ( (not ilc)  and (not powerpulsing) and (temperature) and (not analog) and (digital)):
        mode=0x815A1B40
  
     for di in self.xdaqApps_['StandaloneManager']:
        di.setParameter('ControlRegister','%d' % mode)
        di.updateParameters()

  
