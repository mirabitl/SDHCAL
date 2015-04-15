#!/usr/bin/env python
import logging
from spyne import Application, srpc, ServiceBase, Iterable, UnsignedInteger,String
from spyne.protocol.json import JsonDocument
from spyne.protocol.xml import XmlDocument
from spyne.protocol.csv import Csv
from spyne.protocol.http import HttpRpc
from spyne.server.wsgi import WsgiApplication

import DHCalEventReader as dr
import os,sys
from ROOT import *
import time
import config_tracking as conf

class MonitorAnalysis:
    def __init__(self):
        self.dher=dr.DHCalEventReader();
        self.rootHandler=dr.DCHistogramHandler()
        self.dher.ParseSteering(conf.marlin)
        self.dher.setXdaqShift(24); 

        self.dher.setDropFirstRU(False);
        self.a=dr.ShowerAnalyzer( self.dher,self.rootHandler);
        self.a.presetParameters()
        self.a.setrebuild(False)
        #self.a.setuseSynchronised(conf.useSynch);
        #self.a.setminChambersInTime(conf.minChambersInTime);
        #self.a.settkMinPoint(conf.tkMinPoint);
        #self.a.settkExtMinPoint(conf.tkExtMinPoint);
        #self.a.settkChi2Cut(conf.tkChi2Cut);  
        #self.a.settkExtChi2Cut(conf.tkExtChi2Cut);  
        #self.a.settkDistCut(conf.tkDistCut);  
        #self.a.settkExtDistCut(conf.tkExtDistCut);  
        self.filein=None
        self.dher.registerAnalysis(self.a);

    def writeHistos(self,name):
        self.rootHandler.writeHistograms(name)
    def getHistoList(self):
        return self.rootHandler.getXMLHistoList()
    def getHisto(self,name):
        return self.rootHandler.getXMLHisto(name)
    def startMonitoring(self,directory,nd,nr):
        self.dher.startReadMemory(directory,nd,nr)
    def stopMonitoring(self):
        self.dher.stopReadMemory()
    def startFile(self,file):
        self.dher.startReadFile(file)
 
_ma=MonitorAnalysis()

class MonitorService(ServiceBase):
    @srpc(String, UnsignedInteger,UnsignedInteger, _returns=Iterable(String))
    def startMonitoring(directory,ndif,run):
        """
        Docstrings for service methods do appear as documentation in the
        interface documents. <b>What fun!</b>
        :param name: The name to say hello to
        :param times: The number of times to say hello
        :returns: An array of 'Hello, <name>' strings, repeated <times> times.
        """
        print directory
        print ndif
        print run
        _ma.startMonitoring(directory,ndif,run);
        for i in range(1):
            yield 'Monitoring will start on %s for %d difs and run %d ' % (directory,ndif,run)
    @srpc(String, _returns=Iterable(String))
    def startFile(name):
        """
        Docstrings for service methods do appear as documentation in the
        interface documents. <b>What fun!</b>
        :param name: The name to say hello to
        :param times: The number of times to say hello
        :returns: An array of 'Hello, <name>' strings, repeated <times> times.
        """
        global _ma
        _ma.startFile(name)
        for i in range(1):
            yield 'Monitoring will start on %s ' % (name)
    @srpc( _returns=Iterable(String))
    def stopMonitoring():
        """
        Docstrings for service methods do appear as documentation in the
        interface documents. <b>What fun!</b>
        :param name: The name to say hello to
        :param times: The number of times to say hello
        :returns: An array of 'Hello, <name>' strings, repeated <times> times.
        """
        global _ma
        _ma.stopMonitoring()
        for i in range(1):
            yield 'Monitoring will stop'
    @srpc( _returns=Iterable(String))
    def getHistoList():
        """
        Docstrings for service methods do appear as documentation in the
        interface documents. <b>What fun!</b>
        :param name: The name to say hello to
        :param times: The number of times to say hello
        :returns: An array of 'Hello, <name>' strings, repeated <times> times.
        """
        global _ma
        yield _ma.getHistoList()
    @srpc(String, _returns=String)
    def getHisto(name):
        """
        Docstrings for service methods do appear as documentation in the
        interface documents. <b>What fun!</b>
        :param name: The name to say hello to
        :param times: The number of times to say hello
        :returns: An array of 'Hello, <name>' strings, repeated <times> times.
        """
        global _ma
        yield _ma.getHisto(name)


if __name__=='__main__':
    # Python daemon boilerplate
    from wsgiref.simple_server import make_server
    logging.basicConfig(level=logging.DEBUG)

    # Instantiate the application by giving it:
    #   * The list of services it should wrap,
    #   * A namespace string.
    #   * An input protocol.
    #   * An output protocol.
    application = Application([MonitorService], 'spyne.examples.hello.http',
          # The input protocol is set as HttpRpc to make our service easy to
          # call. Input validation via the 'soft' engine is enabled. (which is
          # actually the the only validation method for HttpRpc.)
          in_protocol=HttpRpc(validator='soft'),

          # The ignore_wrappers parameter to JsonDocument simplifies the reponse
          # dict by skipping outer response structures that are redundant when
          # the client knows what object to expect.
          #out_protocol=XmlDocument()
          #out_protocol=YamlDocument(),
          out_protocol=JsonDocument(ignore_wrappers=False),
      )

    # Now that we have our application, we must wrap it inside a transport.
    # In this case, we use Spyne's standard Wsgi wrapper. Spyne supports 
    # popular Http wrappers like Twisted, Django, Pyramid, etc. as well as
    # a ZeroMQ (REQ/REP) wrapper.
    wsgi_application = WsgiApplication(application)

    # More daemon boilerplate
    server = make_server('lyosdhcal12', 8000, wsgi_application)

    logging.info("listening to http://127.0.0.1:8000")
    logging.info("wsdl is at: http://localhost:8000/?wsdl")

    server.serve_forever()

