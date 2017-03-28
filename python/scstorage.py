import sys

from pydim import  dic_info_service,dic_cmnd_service
from time import sleep
from threading import Timer,Thread, Event,Lock
import MySQLdb as mdb
from  logging import *
from logging.handlers import *
import xmllayout
class perpetualTimer:

   def __init__(self,t,hFunction):
      self.t=t
      self.hFunction = hFunction
      self.thread = Timer(self.t,self.handle_function)

   def handle_function(self):
      self.hFunction()
      self.thread = Timer(self.t,self.handle_function)
      self.thread.start()

   def start(self):
      self.thread.start()

   def cancel(self):
      self.thread.cancel()

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
        self.valid=False
    def set(self,ch,vref,iref,p0,t0):
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
        self.connection=None
        self.bmptname="/BMP183/%s/TEMPERATUREREADVALUES" % bmphost
        self.bmppname="/BMP183/%s/PRESSIONREADVALUES" % bmphost
        self.wiener=wiener
        self.wienernames=[]
        self.pread=0
        self.tread=0
        self.hvchannel={}
        self.hvref={}
        self.registered=False
        self.timer=None
        self.sem=Lock()
        if (self.wiener):
            for i in range(0,7):
                for j in range(0,8):
                    c=i*8+j
                    self.wienernames.append("/WIENER/MODULE%d-CHANNEL%d/%d" % (i,j,c))
                    self.hvchannel[c]=None
                    self.hvref[c]=chamberRef(c,6900.,1000.,1000.,290.)
        self.rootLogger = logging.getLogger('')
        self.rootLogger.setLevel(logging.DEBUG)
        self.socketHandler = logging.handlers.SocketHandler('localhost',
                    logging.handlers.DEFAULT_TCP_LOGGING_PORT)
        self.xmlhandler = xmllayout.RawSocketHandler('lyosdhcal9', 4448)
        self.xmlhandler.setFormatter(xmllayout.XMLLayout())
        self.rootLogger.addHandler(self.socketHandler)
        self.rootLogger.addHandler(self.xmlhandler)
        self.logm=logging.getLogger("MainThread")
        self.logc=logging.getLogger("Checker")
    def setDebug(self):
       self.rootLogger.setLevel(logging.DEBUG)
    def setWarning(self):
       self.rootLogger.setLevel(logging.WARNING)
    def setInfo(self):
       self.rootLogger.setLevel(logging.INFO)

    def loadReferences(self):
        self.logm.info("Loading reference to the main frame")
        for  i in range(0,56):
            if (self.hvref[i].valid):
                self.setVoltage(i,self.hvref[i].vref)
                self.setCurrentLimit(i,self.hvref[i].iref)
    def connect(self):
        if (self.account==None):
            return
        self.logm.debug("DB connection")
        name=self.account[0:self.account.find('/')]
        pwd=self.account[self.account.find('/')+1:self.account.find('@')]
        host= self.account[self.account.find('@')+1:self.account.find(':')]
        db=self.account[self.account.find(':')+1:len(self.account)]
        self.sem.acquire()
        self.connection=mdb.connect(host,name,pwd,db);
    def disconnect(self):
        self.connection.close()
        self.sem.release()
        self.connection=None
    def downloadReferences(self):
        if (self.account==None):
            return
        self.connect()
        self.connection.ping(True)
        cur=self.connection.cursor()
        cur.execute("select HVCHAN,VREF,P0,T0 FROM CHAMBERREF WHERE FIN>NOW() AND DEBUT<NOW();")
        rows = cur.fetchall()
        for row in rows:
            #print row
            self.hvref[int(row[0])].vref=float(row[1])
            self.hvref[int(row[0])].p0=float(row[2])
            self.hvref[int(row[0])].t0=float(row[3])
            self.hvref[int(row[0])].valid=True
            
        cur.close()
        self.disconnect()
    def storeHvChannel(self,i):
       self.logm.debug("storeHvChannel")
       self.connect()
       self.connection.ping(True)
       cur=self.connection.cursor()
       cur.execute("insert into WIENERMON(HVCHAN,VSET,ISET,VOUT,IOUT) VALUES(%d,%f,%f,%f,%f)" % (self.hvchannel[i].channel,self.hvchannel[i].vset,self.hvchannel[i].iset,self.hvchannel[i].vout,self.hvchannel[i].iout))
       self.connection.commit()
       cur.close()
       self.disconnect()
    def wienerHandler(self, *args):
        #print 'HV ',args
        self.logm.debug("wienerHandler")
        self.hvchannel[args[0]]=wienerChannel(args[0],args[1],args[2],args[3],args[4])
        if (self.hvref[args[0]].valid and self.account!=None):
            self.storeHvChannel(args[0])
        #{'Channel':args[0],'Vset':args[1],'Iset':args[2],'Vout':args[3],'Iout':args[4]}
    def storeBMP(self):
        self.logm.debug("storeBMP")
        self.connect()
        self.connection.ping(True)
        cur=self.connection.cursor()
        cur.execute("insert into BMPMON(P,TK) VALUES(%f,%f)" % (self.pread,self.tread))
        self.connection.commit()

        cur.close()
        self.disconnect()
    def BMPPHandler(self, *args):
        self.logm.debug("BMPPhandler")
        #print 'BMP P',args
        self.pread=args[0]
        if (self.account!=None):
            self.storeBMP()
    def BMPTHandler(self, *args):
        self.logm.debug("BMPThandler")
        #print 'BMP T',args
        self.tread=args[0]+273.15
    def register(self):
        dic_info_service(self.bmptname, ScStorage.BMP183TFORMAT,self.BMPTHandler)
        dic_info_service(self.bmppname, ScStorage.BMP183PFORMAT,self.BMPPHandler)
        for x in self.wienernames:
            dic_info_service(x,ScStorage.WIENERFORMAT,self.wienerHandler)
        self.registered=True
    def setPeriod(self,t):
        tuple_args=(t,)
        dic_cmnd_service("/WIENER/SetPeriod",tuple_args,"I:1")
        dic_cmnd_service("/BMP183/%s/SETPERIOD" % self.bmphost,tuple_args,"I:1")
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
        for  i in range(0,56):
            if (self.hvref[i].valid):
                self.hvchannel[i].dump()

    def getPTMean(self):
        self.logc.debug("getPTMean")
        if (self.account==None):
            return []
        self.connect()
        self.connection.ping(True)
        PM=0
        TM=0

        try:
            cur=self.connection.cursor()
            cur.execute("select P,TK FROM BMPMON WHERE TIS BETWEEN NOW() - INTERVAL 10 MINUTE AND NOW()")
            rows = cur.fetchall()
            nrows=len(rows)
            if (len(rows)<10):
                cur.close()
                self.disconnect()
                return []
            for row in rows:
                PM=PM+row[0]
                TM=TM+row[1]
            cur.close()
        except:
            self.logc.error("connection error ")
            self.disconnect()
            return []
        self.disconnect()
        return [PM/nrows,TM/nrows]

    def checkChannel(self,chref,pm,tm):
        self.logc.debug("checkChannel")
        if (self.account==None):
            return
        #chref=self.hvref[i]
        self.connect()
        self.connection.ping(True)
        vset=0
        vout=0

        try:
            cur=self.connection.cursor()
            cur.execute("select VSET,VOUT FROM WIENERMON WHERE  HVCHAN=%d  ORDER BY IDX DESC LIMIT 1" % chref.channel)
            rows = cur.fetchall()
            if (len(rows)==0):
                cur.close()
                self.disconnect()
                return
            for row in rows:
                vset=row[0]
                vout=row[1]
            cur.close()
        except:
            self.logc.error("connection error")
            self.disconnect()
            return
        self.disconnect()
        vexpected=chref.vref*chref.t0/chref.p0*pm/tm
        veffective=vout*chref.p0/chref.t0*tm/pm
        deltav=abs(veffective-chref.vref)
        if (deltav>10 and deltav<200):
            self.logc.info("checkChannel %d : Vout %f Veffective %f Vref %f , the expected value %f will be set automatically " % (self.chref.channel,vout,veffective,chref.vref,vepected))
            self.setVoltage(self.chref.channel,vexpected)
            self.ReadChannel(self.chref.channel)
            self.hvchannel[chref.channel].dump()
        if (deltav>=200 && vout>=5000):
            self.logc.warning("checkChannel %d : Vout %f Veffective %f Vref %f , the expected value %f cannot be set automatically P %f %f T %f %f " % (chref.channel,vout,veffective,chref.vref,vexpected,pm,chref.p0,tm,chref.t0))
         if (deltav>=200 && vout<500):
            self.logc.warning("checkChannel %d : Vout %f the channel is OFF" % (chref.channel,vout)
        return
    def checkSetup(self):
        if (self.account==None):
            self.logc.error("Please connect to Mysql DB first")
            return
        l=self.getPTMean()
        if (len(l)!=2):
            self.logc.warning("not enough P,T measurement")
            return
        pm=l[0]
        tm=l[1]
        for x,y in self.hvref.iteritems():
            if (y.valid):
                self.checkChannel(y,pm,tm)
                
    def startChecking(self,period):
        self.timer=perpetualTimer(period,self.checkSetup)
        self.timer.start()
        self.logc.info(" Check timer started")
    def stopChecking(self):
        if (self.timer !=None):
            self.timer.cancel()
            self.logc.info(" Check timer stopped")
