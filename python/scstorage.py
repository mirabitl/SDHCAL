import sys

from pydim import  dic_info_service,dic_cmnd_service
from time import sleep
from threading import Thread, Event
class wienerChannel:
    def __init__(self,ch,vst,ist,vrd,ird):
        self.channel=ch
        self.vset=vst
        self.iset=ist
        self.vout=vrd
        self.iout=ird
    def dump(self):
        print "%3d %7.1f %7.1f %7.1f %7.1f" %(self.channel,self.vset,self.iset,self.vout,self.iout)
class chamberRef:
    def __init__(self,ch,vref,iref,p0,t0):
        self.channel=ch
        self.vref=vref
        self.iref=iref
        self.p0=p0
        self.t0=t0
class ScStorage:
    WIENERFORMAT="I:1;F:4;"
    BMP183TFORMAT="F"
    BMP183PFORMAT="F"
    def __init__(self,bmphost,wiener=True,account=None):
        self.bmphost=bmphost
        self.account=account
        self.bmptname="/BMP183/%s/TEMPERATUREREADVALUES" % bmphost
        self.bmppname="/BMP183/%s/PRESSIONREADVALUES" % bmphost
        self.wiener=wiener
        self.wienernames=[]
        self.pread=0
        self.tread=0
        self.hvchannel={}
        if (self.wiener):
            for i in range(0,6):
                for j in range(0,8):
                    c=i*8+j
                    self.wienernames.append("/WIENER/MODULE%d-CHANNEL%d/%d" % (i,j,c))
                    self.hvchannel[c]=None
    def wienerHandler(self, *args):
        #print 'HV ',args
        self.hvchannel[args[0]]=wienerChannel(args[0],args[1],args[2],args[3],args[4])
        #{'Channel':args[0],'Vset':args[1],'Iset':args[2],'Vout':args[3],'Iout':args[4]}
    def BMPPHandler(self, *args):
        #print 'BMP P',args
        self.pread=args[0]
    def BMPTHandler(self, *args):
        #print 'BMP T',args
        self.tread=args[0]+273.15
    def register(self):
        dic_info_service(self.bmptname, ScStorage.BMP183TFORMAT,self.BMPTHandler)
        dic_info_service(self.bmppname, ScStorage.BMP183PFORMAT,self.BMPPHandler)
        for x in self.wienernames:
            dic_info_service(x,ScStorage.WIENERFORMAT,self.wienerHandler)

    def setPeriod(self,t):
        tuple_args=(t,)
        dic_cmnd_service("/WIENER/SetPeriod",tuple_args,"I:1")
    def ReadChannel(self,t):
        tuple_args=(t,)
        dic_cmnd_service("/WIENER/ReadChannel",tuple_args,"I:1")

    def setVoltage(self,chan,vset):
        tuple_args=(chan,vset)
        dic_cmnd_service("/WIENER/SetVoltage",tuple_args,"I:1;F:1")
    def setCurrentLimit(self,chan,iset):
        tuple_args=(chan,iset*1E-6)
        dic_cmnd_service("/WIENER/SetCurrent",tuple_args,"I:1;F:1")
    def HVOn(self,chan):
        tuple_args=(chan,1)
        dic_cmnd_service("/WIENER/Switch",tuple_args,"I:2")
    def HVOff(self,chan):
        tuple_args=(chan,0)
        dic_cmnd_service("/WIENER/Switch",tuple_args,"I:2")
    def dump(self):
        print "Pression %.2f  Temperature %.2f C (%.2f) \n" % (self.pread,self.tread-273.15,self.tread)

        print "%3s %7s %7s %7s %7s \n" %("CH","VSET","ISET","VOUT","IOUT")
        for  i in range(0,48):
            self.hvchannel[i].dump()
