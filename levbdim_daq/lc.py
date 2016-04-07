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

# configure all the actions
grp_action = parser.add_mutually_exclusive_group()
grp_action.add_argument('--daq-create',action='store_true',help='Create the RpcDaq object to access DIf/CCC/EVB')
grp_action.add_argument('--jc-create',action='store_true',help='Create the DimJobControlInterface object to control processes')
grp_action.add_argument('--jc-kill',action='store_true',help='kill all controled processes')
grp_action.add_argument('--jc-start',action='store_true',help='start all controled processes described in $DAQCONFIG jsonfile variable')
grp_action.add_argument('--jc-status',action='store_true',help='show the status all controled processes')
grp_action.add_argument('--daq-discover',action='store_true',help='trigger a scan of the DNS othe Daq')
grp_action.add_argument('--daq-setparameters',action='store_true',help='send the paremeters described in $DAQCONFIG file to the DAQ')
grp_action.add_argument('--daq-forceState',action='store_true',help='force the sate name of the Daq with the --state option, ex --forceState --state=DISCOVERED')
grp_action.add_argument('--daq-services',action='store_true',help='Triggers teh download of the DB state, the initialisation of the Zup and of the CCC according to $DAQCONFIG values (compulsary before first initialise)')
grp_action.add_argument('--daq-lvon',action='store_true',help='put Zup LV ON')
grp_action.add_argument('--daq-lvoff',action='store_true',help='put Zup LV OFF')
grp_action.add_argument('--daq-lvstatus',action='store_true',help='LV status')
grp_action.add_argument('--daq-initialise',action='store_true',help=' initialise the DAQ')
grp_action.add_argument('--daq-configure',action='store_true',help=' configure the DAQ')
grp_action.add_argument('--daq-status',action='store_true',help=' display DAQ status of all DIF')
grp_action.add_argument('--daq-state',action='store_true',help=' display DAQ state')
grp_action.add_argument('--daq-evbstatus',action='store_true',help=' display event builder status')
grp_action.add_argument('--daq-startrun',action='store_true',help=' start the run')
grp_action.add_argument('--daq-stoprun',action='store_true',help=' stop the run')
grp_action.add_argument('--trig-status',action='store_true',help=' display trigger counter status')
grp_action.add_argument('--trig-reset',action='store_true',help=' reset trigger counter')
grp_action.add_argument('--trig-pause',action='store_true',help=' trigger soft veto')
grp_action.add_argument('--trig-resume',action='store_true',help=' trigger soft veto release')
grp_action.add_argument('--ecal-pause',action='store_true',help=' trigger soft veto')
grp_action.add_argument('--ecal-resume',action='store_true',help=' trigger soft veto release')
grp_action.add_argument('--trig-spillon',action='store_true',help=' set spill nclock on with --clock=nc (20ns)')
grp_action.add_argument('--trig-spilloff',action='store_true',help=' set spill nclock off with --clock=nc (20ns) ')
grp_action.add_argument('--trig-beam',action='store_true',help=' set spill nclock off with --clock=nc (20ns) ')
grp_action.add_argument('--daq-destroy',action='store_true',help='destroy the DIF readout, back to the PREPARED state')
grp_action.add_argument('--daq-downloaddb',action='store_true',help='download the dbsate specified in --dbstate=state')
grp_action.add_argument('--daq-ctrlreg',action='store_true',help='set the ctrlregister specified with --ctrlreg=register')

grp_action.add_argument('--slc-create',action='store_true',help='Create the DimSlowControl object to control WIENER crate and BMP sensor')

grp_action.add_argument('--slc-initialisesql',action='store_true',help='initiliase the mysql access specified with --account=login/pwd@host:base')

grp_action.add_argument('--slc-loadreferences',action='store_true',help='load in the wiener crate chambers references voltage download from DB')

grp_action.add_argument('--slc-hvstatus',action='store_true',help='display hvstatus of all channel of the wiener crate')

grp_action.add_argument('--slc-ptstatus',action='store_true',help='display the P and T from the BMP183 readout')
grp_action.add_argument('--slc-setperiod',action='store_true',help='set the readout period of Wiener and BMP with --period=second(s)')

grp_action.add_argument('--slc-setvoltage',action='store_true',help='set the voltage V of channel k with --channel=k --voltage=V')
grp_action.add_argument('--slc-setcurrent',action='store_true',help='set the current limit I (microA) of channel k with --channel=k --current=I')
grp_action.add_argument('--slc-hvon',action='store_true',help='set the voltage ON  of channel k with --channel=k ')
grp_action.add_argument('--slc-hvoff',action='store_true',help='set the voltage OFF  of channel k with --channel=k ')
grp_action.add_argument('--slc-store',action='store_true',help='start the data storage in the mysql DB at period p  with --period=p ')
grp_action.add_argument('--slc-store-stop',action='store_true',help='stop the data storage in the mysql DB ')
grp_action.add_argument('--slc-check',action='store_true',help='start the voltage tuning wrt references at period p  with --period=p ')
grp_action.add_argument('--slc-check-stop',action='store_true',help='stop the voltage tuning ')




# Arguments
parser.add_argument('--config', action='store', dest='config',default=None,help='python config file')
parser.add_argument('--socks', action='store', type=int,dest='sockport',default=None,help='use SOCKS port ')
parser.add_argument('--dbstate', action='store', default=None,dest='dbstate',help='set the dbstate')
parser.add_argument('--ctrlreg', action='store', default=None,dest='ctrlreg',help='set the dbstatectrreg in hexa')
parser.add_argument('--version', action='version', version='%(prog)s 1.0')
parser.add_argument('--state', action='store', type=str,default=None,dest='fstate',help='set the rpcdaq state')
parser.add_argument('--channel', action='store',type=int, default=None,dest='channel',help='set the hvchannel')
parser.add_argument('--voltage', action='store',type=float, default=None,dest='voltage',help='set the hv voltage')
parser.add_argument('--current', action='store',type=float, default=None,dest='current',help='set the hv current')

parser.add_argument('--period', action='store',type=int, default=None,dest='period',help='set the tempo period')

parser.add_argument('--clock', action='store',type=int, default=None,dest='clock',help='set the number of 20 ns clock')

parser.add_argument('--account', action='store', default=None,dest='account',help='set the mysql account')

parser.add_argument('-v','--verbose',action='store_true',default=False,help='set the mysql account')

results = parser.parse_args()

#print results
#exit(0)
# Analyse results
if (results.config==None):
    dc=os.getenv("DAQCONFIG","Not Found")
    if (dc=="Not Found"):
        print "please specify a configuration with --config=conf_name"
        exit(0)
    else:
        results.config=dc

# import the configuration
try:
    exec("import %s  as conf" % results.config)
except ImportError:
    raise Exception("cannot import")

# fill parameters 
p_par={}

p_par['dbstate']=conf.dbstate
p_par['zupdevice']=conf.zupdevice
p_par['zupport']=conf.zupport
p_par['filepath']=conf.filepath
p_par['memorypath']=conf.memorypath
p_par['proclist']=conf.proclist
p_par['ctrlreg']=conf.ctrlreg
p_par['dccname']=conf.dccname
p_par['mdccname']=conf.mdccname
p_par['daqhost']=conf.daqhost
p_par['daqport']=conf.daqport
p_par['json']=conf.jsonfile

l_par=json.dumps(p_par,sort_keys=True)

# set the connection mode
if (results.sockport==None):
    sp=os.getenv("SOCKPORT","Not Found")
    if (sp!="Not Found"):
        results.sockport=int(sp)


if (results.sockport !=None):
    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "127.0.0.1", results.sockport)
    socket.socket = socks.socksocket
    #print "on utilise sock",results.sockport
# analyse the command
lcgi={}
r_cmd=None
if (results.daq_create):
    r_cmd='createDaq'
elif(results.jc_create):
    r_cmd='createJobControl'
    lcgi['name']=conf.jsonfile
elif(results.jc_kill):
    r_cmd='jobKillAll'
elif(results.jc_start):
    r_cmd='jobStartAll'
elif(results.jc_status):
    r_cmd='jobStatus'
elif(results.daq_state):
    r_cmd='state'
elif(results.daq_discover):
    r_cmd='Discover'
elif(results.daq_setparameters):
    r_cmd='setParameters'
    lcgi['name']=l_par
elif(results.daq_forceState):
    r_cmd='forceState'
    if (results.fstate!=None):
        lcgi['name']=results.fstate
    else:
        print 'Please specify the state --state=STATE'
        exit(0)
elif(results.daq_services):
    r_cmd='prepareServices'
elif(results.daq_lvon):
    r_cmd='LVON'
elif(results.daq_lvoff):
    r_cmd='LVOFF'
elif(results.daq_lvstatus):
    r_cmd='LVStatus'
elif(results.daq_initialise):
    r_cmd='initialise'
elif(results.daq_configure):
    r_cmd='configure'
elif(results.daq_status):
    r_cmd='status'
elif(results.daq_evbstatus):
    r_cmd='shmStatus'
elif(results.daq_startrun):
    r_cmd='start'
elif(results.daq_stoprun):
    r_cmd='stop'
elif(results.daq_destroy):
    r_cmd='destroy'
elif(results.daq_downloaddb):
    r_cmd='downloadDB'
    if (results.dbstate!=None):
        lcgi['name']=results.dbstate
    else:
        print 'Please specify the state --dbstate=STATE'
        exit(0)
elif(results.daq_ctrlreg):
    r_cmd='setControlRegister'
    if (results.ctrlreg!=None):
        lcgi['value']=int(results.ctrlreg,16)
    else:
        print 'Please specify the value --ctrlreg=0xX######'
        exit(0)
elif(results.trig_status):
    r_cmd='triggerStatus'
elif(results.trig_beam):
    r_cmd='triggerBeam'
    if (results.clock!=None):
        lcgi['nclock']=results.clock
    else:
        print 'Please specify the number of clock --clock=xx'
        exit(0)
elif(results.trig_spillon):
    r_cmd='triggerSpillOn'
    if (results.clock!=None):
        lcgi['nclock']=results.clock
    else:
        print 'Please specify the number of clock --clock=xx'
        exit(0)
elif(results.trig_spilloff):
    r_cmd='triggerSpillOff'
    if (results.clock!=None):
        lcgi['nclock']=results.clock
    else:
        print 'Please specify the number of clock --clock=xx'
        exit(0)
elif(results.ecal_pause):
    r_cmd='pauseEcal'
elif(results.ecal_resume):
    r_cmd='resumeEcal'
elif(results.trig_reset):
    r_cmd='resetTrigger'
elif(results.trig_pause):
    r_cmd='pause'
elif(results.trig_resume):
    r_cmd='resume'
elif(results.slc_create):
    r_cmd='createSlowControl'
elif(results.slc_initialisesql):
    r_cmd='initialiseDB'
    if (results.account!=None):
        lcgi['account']=results.account
    else:
        print 'Please specify the MYSQL account --account=log/pwd@host:base'
        exit(0)
elif(results.slc_loadreferences):
    r_cmd='loadReferences'
elif(results.slc_hvstatus):
    r_cmd='hvStatus'
elif(results.slc_ptstatus):
    r_cmd='PT'
elif(results.slc_setperiod):
    r_cmd='setReadoutPeriod'
    if (results.period!=None):
        lcgi['period']=results.period
    else:
        print 'Please specify the period --period=second(s)'
        exit(0)
elif(results.slc_setvoltage):
    r_cmd='setVoltage'
    if (results.channel!=None):
        lcgi['channel']=results.channel
    else:
        print 'Please specify the channel --channel=#'
        exit(0)
    if (results.voltage!=None):
        lcgi['V']=results.voltage
    else:
        print 'Please specify the voltage --voltage=V'
        exit(0)
elif(results.slc_setcurrent):
    r_cmd='setCurrentLimit'
    if (results.channel!=None):
        lcgi['channel']=results.channel
    else:
        print 'Please specify the channel --channel=#'
        exit(0)
    if (results.current!=None):
        lcgi['I']=results.current
    else:
        print 'Please specify the current limit --current=I (microA)'
        exit(0)
elif(results.slc_hvon):
    r_cmd='HVON'
    if (results.channel!=None):
        lcgi['channel']=results.channel
    else:
        print 'Please specify the channel --channel=#'
        exit(0)
elif(results.slc_hvoff):
    r_cmd='HVOFF'
    if (results.channel!=None):
        lcgi['channel']=results.channel
    else:
        print 'Please specify the channel --channel=#'
        exit(0)
elif(results.slc_store):
    r_cmd='startStorage'
    if (results.period!=None):
        lcgi['period']=results.period
    else:
        print 'Please specify the period --period=second(s)'
        exit(0)
elif(results.slc_check):
    r_cmd='startCheck'
    if (results.period!=None):
        lcgi['period']=results.period
    else:
        print 'Please specify the period --period=second(s)'
        exit(0)
elif(results.slc_store_stop):
    r_cmd='stopStorage'
elif(results.slc_check_stop):
    r_cmd='stopCheck'

#print r_cmd
#print lcgi


def sendcommand2(command,host=p_par["daqhost"],port=p_par['daqport'],lq=None):
   global results 
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
           #print myurl
           req=urllib2.Request(myurl)
           r1=urllib2.urlopen(req)

           return r1.read()
   else:
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       saction = '/%s' % command
       myurl=myurl+saction
       #print myurl
       req=urllib2.Request(myurl)
       r1=urllib2.urlopen(req)
       if (command=="status" and not results.verbose):
           s=r1.read()
           sj=json.loads(s)
           ssj=json.loads(sj["statusResponse"]["statusResult"][0])
#           for x in ssj:
#             for d in x["difs"]:
#                print '#%4d %5x %6d %12d %12d %s %s ' % (d["id"],d["slc"],d["gtc"],d["bcid"],d["bytes"],d["state"],x["name"])
           print "\033[1m %4s %5s %6s %12s %12s %15s  %s \033[0m" % ('DIF','SLC','EVENT','BCID','BYTES','SERVER','STATUS')

           for d in ssj:
               #print d
               #for d in x["difs"]:
               print '#%4d %5x %6d %12d %12d %15s %s ' % (d["id"],d["slc"],d["gtc"],d["bcid"],d["bytes"],d["host"],d["state"])
       elif (command=="jobStatus" and not results.verbose ):
           s=r1.read()
           sj=json.loads(s)
           ssj=json.loads(sj["jobStatusResponse"]["jobStatusResult"][0])
           print "\033[1m %6s %15s %25s %20s \033[0m" % ('PID','NAME','HOST','STATUS')
           for x in ssj:
               if (x['DAQ']=='Y'):
                   print "%6d %15s %25s %20s" % (x['PID'],x['NAME'],x['HOST'],x['STATUS'])
       elif (command=="hvStatus" and not results.verbose):
           s=r1.read()
           sj=json.loads(s)
           ssj=json.loads(sj["hvStatusResponse"]["hvStatusResult"][0])
           print "\033[1m %5s %10s %10s %10s %10s \033[0m" % ('Chan','VSET','ISET','VOUT','IOUT')
           for x in ssj:
               print "#%.4d %10.2f %10.2f %10.2f %10.2f" % (x['channel'],x['vset'],x['iset'],x['vout'],x['iout'])

       elif (command=="LVStatus" and not results.verbose ):
           s=r1.read()
           sj=json.loads(s)
           ssj=json.loads(sj["LVStatusResponse"]["LVStatusResult"][0])
           print "\033[1m %10s %10s %10s \033[0m" % ('VSET','VOUT','IOUT')
           print " %10.2f %10.2f %10.2f" % (ssj['vset'],ssj['vout'],ssj['iout'])
           #for x in ssj:
           #    print "#%.4d %10.2f %10.2f %10.2f %10.2f" % (x['channel'],x['vset'],x['iset'],x['vout'],x['iout'])
       elif (command=="shmStatus" and not results.verbose):
           s=r1.read()
           sj=json.loads(s)
           ssj=json.loads(sj["shmStatusResponse"]["shmStatusResult"][0])
           print "\033[1m %10s %10s \033[0m" % ('Run','Event')
           print " %10d %10d " % (ssj['run'],ssj['event'])
       elif (command=="triggerStatus" and not results.verbose):
           s=r1.read()
           sj=json.loads(s)
           ssj=json.loads(sj["triggerStatusResponse"]["triggerStatusResult"][0])
           print "\033[1m %10s %10s %10s %10s %12s %12s %10s %10s %10s \033[0m" % ('Spill','Busy1','Busy2','Busy3','SpillOn','SpillOff','Beam','Mask','EcalMask')
           print " %10d %10d %10d %10d  %12d %12d %12d %10d %10d " % (ssj['spill'],ssj['busy1'],ssj['busy2'],ssj['busy3'],ssj['spillon'],ssj['spilloff'],ssj['beam'],ssj['mask'],ssj['ecalmask'])

       else:
          print r1.read()
          return r1.read()

       
       #print r1.status, r1.reason

if (r_cmd==None):
    print "No command given"
    exit(0)
if (len(lcgi)!=0):
    sendcommand2(r_cmd,lq=lcgi)
else:
    sendcommand2(r_cmd)
exit(0)
