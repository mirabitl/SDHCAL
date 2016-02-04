#!/usr/bin/python
import subprocess
import MySQLdb as mdb
import MySQLdb
import glob
import os,shutil

class sdSqlCreator:
    def __init__(self,runlist,testname):
        self._runfile=runlist
        self._name=testname
        self._db=MySQLdb.connect("lyosdhcal10","acqilc","RPC_2008","GEOMETRY")
        fo=open(self._runfile)
        runa=fo.readlines()
        fo.close()
        self._runs=[]
        for x in runa:
            self._runs.append(int(x))
        #print self._runs
        self._runinfo=[]
        c=self._db.cursor()
        c.execute('select IDX FROM VERSIONS WHERE TESTNAME="%s"' % testname)
        res=c.fetchall()
        self._versionid=0
        for rw in res:
            self._versionid=int(rw[0])
        for x in self._runs:
            c.execute('SELECT RUN,CONFIGURATION,ENERGY,BEAM,DATE FROM LOGBOOK WHERE RUN=%d' % x)
            res=c.fetchall()
            for rw in res:
                run=int(rw[0])
                cfg=rw[1]
                e=float(rw[2])
                bea=rw[3]
                dat=rw[4]
                self._runinfo.append((run,dat,cfg,e,bea,self._versionid,[]))
        c.close()
        #print self._runinfo
        #print self._versionid
    def listFiles(self,directory,ctype,pattern='*'):
        for x in self._runinfo:
            run=x[0]
            stl="%s/%s/%s/%s%d*.slcio" % (directory,self._name,ctype,pattern,run)
            #print stl
            lf=glob.glob(stl)
            for f in lf:
                x[6].append(f)
    def insertRuns(self,replace=False):
        c=self._db.cursor()
        if replace:
            for x in self._runinfo:
                cmd='DELETE FROM RUNS WHERE RUN=%d' % x[0]
                c.execute(cmd)
            self._db.commit()
        print "DELETE COMPLETED"
        for x in self._runinfo:
            cmd='INSERT INTO RUNS (RUN,DEBUT,ENERGY,BEAM,CONFIGURATION,VERSIONID)  VALUES(%d,"%s",%f,"%s","%s",%d)' % (x[0],str(x[1]),x[3],x[4],x[2],x[5])
            c.execute(cmd)
        self._db.commit()
        c.close()
    def insertFiles(self):
        c=self._db.cursor()
        for x in self._runinfo:
            lf=x[6]
            run=x[0]
            vers=x[5]
            for f in lf:
                seq=int(os.path.basename(f).split('_')[3].split('.')[0])
                fu=int(os.path.basename(f).split('_')[2][1:])
                prefix=os.path.basename(f).split('_')[0]
                compress=0
                if prefix[:3] == 'TDH':
                    compress=2
                if prefix[:2] == 'SO':
                    compress=1

                cmd='INSERT INTO FILES (LOCATION,COMPRESS,RUN,SQUENCE,FILTERUNIT,VERSIONID)  VALUES("%s",%d,%d,%d,%d,%d)' % (f,compress,run,seq,fu,vers)
                print cmd
                c.execute(cmd)
        self._db.commit()
        c.close()
    def buildList(self,testname,base="/data/srv02/RAID6"):
        db=MySQLdb.connect("lyosdhcal10","acqilc","RPC_2008","GEOMETRY")
        c=db.cursor()
        
        """ 
        #SPS Beam test SDHCAL
        c.execute('SELECT RUN,CONFIGURATION,ENERGY,BEAM,DATE FROM LOGBOOK WHERE DATE>=(SELECT DEBUT FROM VERSIONS WHERE TESTNAME="%s") AND DATE<=(SELECT FIN FROM VERSIONS WHERE TESTNAME="%s") AND BEAM !="NO" AND ENERGY!=0 AND CONFIGURATION NOT LIKE "%%GIF%%" ORDER BY RUN' % (testname,testname))
        """
        c.execute('SELECT RUN,CONFIGURATION,ENERGY,BEAM,DATE FROM LOGBOOK WHERE DATE>=(SELECT DEBUT FROM VERSIONS WHERE TESTNAME="%s") AND DATE<=(SELECT FIN FROM VERSIONS WHERE TESTNAME="%s") AND BEAM !="NO" AND ENERGY!=0 AND CONFIGURATION NOT LIKE "%%Dome%%" ORDER BY RUN' % (testname,testname))
        res=c.fetchall()
        for rw in res:
            run=int(rw[0])
            cfg=rw[1]
            e=float(rw[2])
            bea=rw[3]
            dat=rw[4]
            stl="%s/NASCopy/*%d*.slcio" % (base,run)
            lf=glob.glob(stl)
            if (len(lf)==0):
                print "Missing",run,cfg,e,bea,dat,lf
                continue
            for f in lf:
                fn=os.path.splitext(os.path.basename(f))
                lfn=fn[0].split('_')
                if (lfn[0]=='DHCAL' and len(lfn)>3):
                    fu=99
                    seq=99
                    compress=0
                    if (lfn[2][0:1]=='I' and len(lfn)==4):
                    #print fn,'FILTERUNIT',lfn[2][1:2],lfn[3]
                        fu=int(lfn[2][1:2])
                        try:
                            seq=int(lfn[3])
                        except:
                            seq=99
                            print "ERROR",lfn
                        if (fu>4):
                    #print fu,seq,fn
                            compress=1
                
                    print f,run,fu,seq,compress
                    if (compress):
                        shutil.move(f,"%s/%s/Compressed" % (base,testname))
                    else:
                        shutil.move(f,"%s/%s/Raw" % (base,testname))
                else:
                    oth=1
                    print "\t \t Other format ",
                    shutil.move(f,"%s/%s/Other" % (base,testname))
