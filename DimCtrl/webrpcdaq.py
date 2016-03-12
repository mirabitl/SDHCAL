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
import os,sys,json
import socket
import time
_wdd=None

class wddService(ServiceBase):
    @srpc(_returns=Iterable(String))
    def createDaq():    
        global _wdd
        _wdd=dc.RpcDaq()
        yield 'Daq is created' 

    @srpc( _returns=Iterable(String))
    def Discover():
        global _wdd
        _wdd.scandns();
        yield 'DIM DNS is browsed %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc( String,_returns=Iterable(String))
    def setParameters(name):
        global _wdd
        print "On a recu ",name
        print "et on appelle",_wdd
        _wdd.setParameters(name)
        yield 'parameters set %s (%s)' % (_wdd.msg(),_wdd.state())

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

