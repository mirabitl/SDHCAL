#!/usr/bin/python
import sys, time
import pydim
import biblioSNMP as HT

svc={}
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
    
    return (chan,HT.getOutputVoltage(imod,ichan),HT.getOutputCurrentLimit(imod,ichan)*1E6,HT.getOutputMeasurementSenseVoltage(imod, ichan),HT.getOutputMeasurementCurrent(imod,ichan)*1E6)
def callback2(tag):
    # Calculate the value
    # ...
    # Remember, the callback function must return a tuple
    return ( "hello world", )
def setvoltage(*args):
    print'Server: I am an unbound dummy function. I\'ve received', args
    l=args
    print l[0],l[1]
    chan=int(l[0][0])
    V=float(l[0][1])
    print chan,V
    imod=chan/8
    ichan=chan%8
    HT.setOutputVoltage(imod,ichan,V)
def switch(*args):
    print'Server: I am an unbound dummy function. I\'ve received', args
    l=args
    print l[0],l[1]
    chan=int(l[0][0])
    S=int(l[0][1])
    print chan,S
    imod=chan/8
    ichan=chan%8
    if S==0:
        HT.setOutputSwitch(imod,ichan,HT.OFF)
    else:
        HT.setOutputSwitch(imod,ichan,HT.ON)
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
pydim.dis_add_cmnd('SetVoltage', CMND3FORMAT, setvoltage, 101)
pydim.dis_add_cmnd('Switch',"I:2", switch, 102)

# Start the DIM server
pydim.dis_start_serving('lyoilchv01-control')
period=20
while True:
    # Update the service periodically
    for x,y in svc.iteritems():
        pydim.dis_update_service(y)
    time.sleep(period)
