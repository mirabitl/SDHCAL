#!/usr/bin/python
import sys, time,datetime
import pydim
import biblioSNMP as HT
import threading
svc={}
sem=threading.Lock()
period=10

def readchannel(*args):
    print'Read Channel ', args
    chan=args[0][0]
    if (chan>56):
        for x,y in svc.iteritems():
            pydim.dis_update_service(y)
    else:
        pydim.dis_update_service(svc[chan])
def setperiod(*args):
    print'Set Period ', args
    global period
    period=args[0][0]
    print period
def readhv_callback(tag):
    """
    Service callbacks are functions (in general, Python callable objects)
    that take one argument: the DIM tag used when the service was added,
    and returns a tuple with the values that corresponds to the service
    parameters definition in DIM.
    """
    # Calculate the value of the server
    # ...
    
    chan=tag
    imod=chan/8
    ichan=chan%8
    sem.acquire()
    #print "reading %d %d \n" % (imod,ichan)
    l=(chan,HT.getOutputVoltage(imod,ichan),HT.getOutputCurrentLimit(imod,ichan)*1E6,HT.getOutputMeasurementSenseVoltage(imod, ichan),HT.getOutputMeasurementCurrent(imod,ichan)*1E6)
    sem.release()
    return l
def setvoltage(*args):
    print'Set Voltage ', args
    l=args
    print l[0],l[1]
    chan=int(l[0][0])
    V=float(l[0][1])
    #print chan,V
    imod=chan/8
    ichan=chan%8
    sem.acquire()
    HT.setOutputVoltage(imod,ichan,V)
    sem.release()
    print 'Set Voltage completed'
def setcurrent(*args):
    print 'Set Current ', args
    l=args
    print l[0],l[1]
    chan=int(l[0][0])
    I=float(l[0][1])
    #print chan,I
    imod=chan/8
    ichan=chan%8
    sem.acquire()
    HT.setOutputCurrentLimit(imod,ichan,I)
    sem.release()
    print 'Set Current completed'
def switch(*args):
    print'Switch', args
    l=args
    print l[0],l[1]
    chan=int(l[0][0])
    S=int(l[0][1])
    print chan,S
    imod=chan/8
    ichan=chan%8
    sem.acquire()
    if S==0:
        HT.setOutputSwitch(imod,ichan,HT.OFF)
    else:
        HT.setOutputSwitch(imod,ichan,HT.ON)
    sem.release()
    print 'Switch completed '
# The function dis_add_service is used to register the service in DIM

for  imod in range(0,6):
    for ichan in range(0,8):
        # 
        svname="/WIENER/MODULE%d-CHANNEL%d/%d" % (imod,ichan,imod*8+ichan)
        srv= pydim.dis_add_service(svname, "I:1;F:4;", readhv_callback,imod*8+ichan )
        svc[imod*8+ichan]=srv
        print svname

# Register another service



# A service must be updated before using it.
for x,y in svc.iteritems():
    pydim.dis_update_service(y)



#
CMND3FORMAT="I:1;F"
pydim.dis_add_cmnd('/WIENER/SetVoltage', CMND3FORMAT, setvoltage,0)
pydim.dis_add_cmnd('/WIENER/SetCurrent', CMND3FORMAT, setcurrent,0)
pydim.dis_add_cmnd('/WIENER/Switch',"I:2", switch,0)
pydim.dis_add_cmnd('/WIENER/SetPeriod',"I:1", setperiod,0)
pydim.dis_add_cmnd('/WIENER/ReadChannel',"I:1",readchannel,0)

# Start the DIM server
pydim.dis_start_serving('lyoilchv01-control')

while True:
    # Update the service periodically
    print "Updating ",period,datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    for x,y in svc.iteritems():
        pydim.dis_update_service(y)

    time.sleep(period)
