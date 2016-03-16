#!/usr/bin/env python
import logging
from spyne import Application, srpc, ServiceBase, Iterable, UnsignedInteger,String,Float,Unicode
from spyne.protocol.json import JsonDocument
from spyne.protocol.xml import XmlDocument
from spyne.protocol.csv import Csv
from spyne.protocol.http import HttpRpc
from spyne.server.wsgi import WsgiApplication
from spyne.model.complex import ComplexModelMeta
import LSDHCALDimCtrl as dc
import Ldimjc
import os,sys,json
import socket
import time
_wdd=None
_wjobc=None
_wsl=None

class wddService(ServiceBase):
    @srpc(_returns=Iterable(String))
    def createDaq():    
        global _wdd
        _wdd=dc.RpcDaq()
        yield 'Daq is created'
        

    @srpc( String, _returns=Iterable(String))
    def setParameters(name):
        global _wdd
        print "On a recu ",name
        if (name!=None):
            _wdd.setParameters(name)
            sm=_wdd.msg()
            ss=_wdd.state()
            yield 'parameters set %s (%s)' % (sm,ss)
        else:
            yield 'Cannot Set'
    @srpc( String, _returns=Iterable(String))
    def setDBState(name):
        global _wdd
        print "On a recu ",name
        if (name!=None):
            _wdd.setDBState(name)
            sm=_wdd.msg()
            ss=_wdd.state()
            yield 'DBState set %s (%s)' % (sm,ss)
        else:
            yield 'Cannot Set DB'
    @srpc( UnsignedInteger, _returns=Iterable(String))
    def setControlRegister(value):
        global _wdd
        print "On a recu ",value
        if (value!=None):
            _wdd.setControlRegister(value)
            sm=_wdd.msg()
            ss=_wdd.state()
            yield 'ControlReg set %s (%s)' % (sm,ss)
        else:
            yield 'Cannot Set Control register'
    
    @srpc( _returns=Iterable(String))
    def downloadDB():
        global _wdd
        _wdd.downloadDB();
        yield 'DB download %s (%s)' % (_wdd.msg(),_wdd.state())

        
    @srpc( _returns=Iterable(String))
    def Discover():
        global _wdd
        _wdd.scandns();
        yield 'DIM DNS is browsed %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc(String, _returns=String)
    def createJobControl(name):
        global _wjobc
        #print "On a recu ",name
        _wjobc=Ldimjc.DimJobInterface()
        _wjobc.loadJSON(name)
        yield 'JOB Control interface  created '
        #print _wjobc
        #yield 'JOB Control interface  created '

    @srpc( _returns=String)
    def jobStatus():
       global _wjobc
       if (_wjobc!=None):
           _wjobc.status()
           time.sleep(2)
           _wjobc.list()
           s=_wjobc.processStatusList()
           yield s
       else:
           yield "No job control found" 

    @srpc( _returns=String)
    def jobKillAll():
       global _wjobc
       if (_wjobc!=None):
           _wjobc.clearAllJobs()
           yield "All jobs killed"
       else:
           yield "No job control found"

    @srpc( _returns=String)
    def jobStartAll():
       global _wjobc
       if (_wjobc!=None):
           print _wjobc
           _wjobc.startJobs("ALL")
           time.sleep(2)
           yield "All jobs started"
       else:
           yield "No job control found"

        
    @srpc( String,_returns=Iterable(String))
    def forceState(name):
        global _wdd
        print "On a recu ",name
        print "et on appelle",_wdd
        _wdd.publishState(name)
        yield 'State is set to %s ' % name  

    @srpc( _returns=Iterable(String))
    def prepareServices():
        global _wdd
        _wdd.prepareServices();
        yield 'Services ready %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc( _returns=Iterable(String))
    def status():
       global _wdd
       yield _wdd.status()
       
    @srpc( _returns=Iterable(String))
    def shmStatus():
       global _wdd
       yield _wdd.shmStatus()

    @srpc( _returns=Iterable(String))
    def state():
       global _wdd
       yield _wdd.state()


    @srpc( _returns=Iterable(String))
    def initialise():
       global _wdd
       _wdd.initialise()
       yield 'SDHCAL is Initialised %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc( _returns=Iterable(String))
    def configure():
       global _wdd
       _wdd.configure()
       yield 'SDHCAL is configured %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc( _returns=Iterable(String))
    def start():
       global _wdd
       _wdd.start()
       yield 'Run is Started %s (%s)' % (_wdd.msg(),_wdd.state())


    @srpc( _returns=Iterable(String))
    def stop():
       global _wdd
       _wdd.stop()
       yield 'Run is stopped %s (%s)' % (_wdd.msg(),_wdd.state())


    @srpc( _returns=Iterable(String))
    def destroy():
       global _wdd
       _wdd.destroy()
       yield 'Daq is Destroyed %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc( _returns=Iterable(String))
    def LVOFF():
       global _wdd
       _wdd.LVOFF()
       yield 'Lv is Off'


    @srpc( _returns=Iterable(String))
    def LVON():
       global _wdd
       _wdd.LVON()
       yield 'Lv is On'

    @srpc(_returns=Iterable(String))
    def createSlowControl():    
        global _wsl
        _wsl=dc.DimSlowControl()
        yield 'Slow Control is created'
        
    @srpc(_returns=Iterable(String))
    def hvStatus():
        global _wsl
        if (_wsl!=None):
            _wsl.readChannel(99)
            yield _wsl.hvinfoCrate()
        else:
            yield " No slow control access"

    @srpc(UnsignedInteger,_returns=Iterable(String))
    def setReadoutPeriod(period):
        global _wsl
        if (_wsl!=None):
            _wsl.setReadoutPeriod(period)
            yield 'Period set to %d' % period
        else:
            yield " No slow control access"

    @srpc(UnsignedInteger,_returns=Iterable(String))
    def startStorage(period):
        global _wsl
        if (_wsl!=None):
            _wsl.startStore(period)
            yield 'Storage started with period set to %d' % period
        else:
            yield " No slow control access"

    @srpc(UnsignedInteger,_returns=Iterable(String))
    def startCheck(period):
        global _wsl
        if (_wsl!=None):
            _wsl.startCheck(period)
            yield 'HV check started with period set to %d' % period
        else:
            yield " No slow control access"

    @srpc(_returns=Iterable(String))
    def stopStorage():
        global _wsl
        if (_wsl!=None):
            _wsl.stopStore()
            yield ' Storage stopped'
        else:
            yield " No slow control access"
            
    @srpc(_returns=Iterable(String))
    def stopCheck():
        global _wsl
        if (_wsl!=None):
            _wsl.stopCheck()
            yield ' HV check stopped'
        else:
            yield " No slow control access"

    @srpc(_returns=Iterable(String))
    def PT():
        global _wsl
        if (_wsl!=None):
            yield ' P=%f T=%f' % (_wsl.pression(),_wsl.temperature())
        else:
            yield " No slow control access"


    @srpc(String,_returns=Iterable(String))
    def initialiseDB(account):
        global _wsl
        if (_wsl!=None):
            _wsl.initialiseDB(account)
            yield 'mySql initialise with %s' % account
        else:
            yield " No slow control access"

    @srpc(_returns=Iterable(String))
    def loadReferences():
        global _wsl
        if (_wsl!=None):
            _wsl.loadReferences()
            yield ' HV references load to the wiener crate'
        else:
            yield " No slow control access"


    @srpc(UnsignedInteger,Float, _returns=Iterable(String))
    def setVoltage(channel,V):
        global _wsl
        if (_wsl!=None):
            _wsl.setVoltage(channel,V)
            yield " channel %d set to %f " % (channel,V)
        else:
            yield " No slow control access"
    @srpc(UnsignedInteger,Float, _returns=Iterable(String))
    def setCurrentLimit(channel,I):
        global _wsl
        if (_wsl!=None):
            _wsl.setCurrentLimit(channel,I)
            yield " channel %d set current to %f " % (channel,I)
        else:
            yield " No slow control access"
        
    @srpc(UnsignedInteger, _returns=Iterable(String))
    def HVON(channel):
        global _wsl
        if (_wsl!=None):
            _wsl.HVON(channel)
            yield " channel %d is ON " % (channel)
        else:
            yield " No slow control access"

    @srpc(UnsignedInteger, _returns=Iterable(String))
    def HVOFF(channel):
        global _wsl
        if (_wsl!=None):
            _wsl.HVOFF(channel)
            yield " channel %d is OFF " % (channel)
        else:
            yield " No slow control access"

            
if __name__=='__main__':
    # Python daemon boilerplate
    from wsgiref.simple_server import make_server
    logging.basicConfig(level=logging.INFO)

    # Instantiate the application by giving it:
    #   * The list of services it should wrap,
    #   * A namespace string.
    #   * An input protocol.
    #   * An output protocol.
    application = Application([wddService], 'spyne.examples.hello.http',
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
    
    server = make_server(socket.gethostname(), 8100, wsgi_application)

    logging.info("listening to http://lyosdhcal12:8100")
    logging.info("wsdl is at: http://lyosdhcal12:8100/?wsdl")

    server.serve_forever()

