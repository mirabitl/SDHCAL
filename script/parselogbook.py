from lxml import etree
from ilcconfdb import *
import RocDescription as rd
import time

tree = etree.parse("Desktop/logbooksummary.xml")
fl=open("logbooksummary.sql","w+")
DBInit_init()
for user in tree.xpath("/ELOG_LIST/ENTRY"):
    mid=int(user.xpath("MID")[0].text)
    run=int(user.xpath("Run")[0].text)
    daq=user.xpath("DAQ")[0].text.split(" ")[0]
    conf=daq
    if (daq[0:6]=="SDHCAL"):
        d=Daq_getDaq(daq)
        xx=d.getXML()
        i0=xx.find("OracleSetupName")
        i1=xx[i0:len(xx)].find(">")
        i2=xx[i0:len(xx)].find("<")
        conf=xx[i0+i1+1:i0+i2]

        #conf=d.getConfigName()
    heure=user.xpath("DATE")[0].text.split(",")[1].split("+0")[0]
    heure=time.strftime('%Y-%m-%d %H:%M:%S',time.strptime(heure," %d %b %Y %H:%M:%S "))
    Energie=0
    if (user.xpath("Energie")[0].text !=None):
        Energie=float(user.xpath("Energie")[0].text.split("GeV")[0])
    Faisceau="NO"
    if (user.xpath("Faisceau")[0].text !=None):
        Faisceau=user.xpath("Faisceau")[0].text.upper()
    print mid,run,daq,conf,heure,Energie,Faisceau
    fl.write("INSERT INTO LOGBOOK (MID,RUN,DAQ,CONFIGURATION,DATE,ENERGY,BEAM) VALUES(%d,%d,\"%s\",\"%s\",\"%s\",%f,\"%s\");\n" % (mid,run,daq,conf,heure,Energie,Faisceau))
fl.close()
