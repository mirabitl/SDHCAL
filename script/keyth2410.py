#!/usr/bin/python
import time
import serial
import os,sys
if len(sys.argv) > 2:
    fname=sys.argv[1]
    thick=float(sys.argv[2] )
else:
    fname='summaryiv'
    thick=0.2

surf=4.33736   



    
# configure the serial connections (the parameters differs on the device you are connecting to)

ser = serial.Serial("/dev/ttyUSB0")  # open first serial port
print ser.portstr       # check which port was really used
ser.baudrate = 9600

ser.bytesize = serial.EIGHTBITS #number of bits per bytes

ser.parity = serial.PARITY_NONE #set parity check: no parity

ser.stopbits = serial.STOPBITS_ONE #number of stop bits

    #ser.timeout = None          #block read

ser.timeout = 1            #non-block read

ser.write("*idn?\r\n")      # write a string

print ser.readline()

ser.write(':sour:volt:mode fix\r\n')
ser.write(':form:elem?\r\n')
print ser.readline()
ser.write(':route:term rear\r\n')
ser.write(':outp on\r\n')

lti=time.localtime()
fl=open("%s_%d_%d_%d_%d_%d.txt" % (fname,lti.tm_year,lti.tm_mon,lti.tm_mday,lti.tm_hour,lti.tm_min),"w")

T0=0

for v in range(100,1001,100):
    ser.write(':sour:volt %f\r\n' % v)
    time.sleep(1)
    for i in range(0,120):
        ser.write(':read?\r\n')
        vread=ser.readline().split(',')
        VM=float(vread[0])
        IM=float(vread[1])
        T=float(vread[3])
        if (T0 == 0):
            T0=T-1;
        RM=VM/IM
        RHO=RM*surf/thick
        print "%d,%g,%g,%g,%g,%f\n" % (i,VM,IM,RM,RHO,T-T0)
        fl.write("%d,%g,%g,%g,%g,%f\n" % (i,VM,IM,RM,RHO,T-T0))
        time.sleep(1)


for v in range(1000,1,-100): 
    ser.write(':sour:volt %f\r\n' % v)
    time.sleep(1)
    for i in range(0,120):
        ser.write(':read?\r\n')
        vread=ser.readline().split(',')
        VM=float(vread[0])
        IM=float(vread[1])
        T=float(vread[3])
        if (T0 == 0):
            T0=T-1;
        RM=VM/IM
        RHO=RM*surf/thick
        print "%d,%g,%g,%g,%g,%f\n" % (i,VM,IM,RM,RHO,T-T0)
        fl.write("%d,%g,%g,%g,%g,%f\n" % (i,VM,IM,RM,RHO,T-T0))
        time.sleep(1)


ser.write(':sour:volt 0.0\r\n')
ser.write(':outp off\r\n')
ser.close()
fl.close()

