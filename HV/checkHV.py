#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import time
import subprocess
import biblioSNMP as HT

"""
if (HT.getSysMainSwitch() == "on"):
	print "Vérification de l'alimentation... OK !"
	
	HT.setOutputVoltage(HT.MODULE_0, HT.VOIE_0, 2000)
	print HT.getOutputVoltage(HT.MODULE_0, HT.VOIE_0)
	
	HT.setOutputSwitch(HT.MODULE_0, HT.VOIE_0, HT.ON)
	print HT.getOutputSwitch(HT.MODULE_0, HT.VOIE_0)
	
	HT.setOutputCurrentLimit(HT.MODULE_0, HT.VOIE_0, 0.0002)
	print HT.getOutputCurrentLimit(HT.MODULE_0, HT.VOIE_0)
	
	print HT.getOutputMeasurementSenseVoltage(HT.MODULE_0, HT.VOIE_0)
	print HT.getOutputMeasurementCurrent(HT.MODULE_0, HT.VOIE_0)
	
	HT.setOutputVoltageRiseRate(HT.MODULE_0, HT.VOIE_0, -50)
	print HT.getOutputVoltageRiseRate(HT.MODULE_0, HT.VOIE_0)
	
	print HT.getOutputStatus(HT.MODULE_0, HT.VOIE_0)
	
	print HT.setGroupsSwitch(GROUPSSWITCH_ALL_OFF)

else:
	print "Les modules HT ne sont pas sous tension, fin du programme."
"""
import MySQLdb as mdb



if (HT.getSysMainSwitch() == "on"):
	print "Vérification de l'alimentation... OK !"
	
	print "Initialisation des modules"
#	for i in range(0, 8):
#		HT.setOutputVoltage(HT.MODULE_0, i, 0)
		
#	HT.setOutputVoltage(HT.MODULE_0, HT.VOIE_0, 0)
#	HT.setOutputSwitch(HT.MODULE_0, HT.VOIE_0, HT.OFF)
	
	while(1):
            con = mdb.connect('localhost', 'acqilc', 'RPC_2008', 'SLOWAVRIL2015');
            cur=con.cursor();

            # fetch currentP and T
            cur.execute("select ID,P,T FROM PT ORDER BY ID DESC LIMIT 10")


            rows = cur.fetchall()
            P=0
            T=0
            for row in rows:
                # print row
                # bug in insertion
                P=P+row[2]
                T=T+row[1]+273.15
            P=P/10
            T=T/10
            print "Current Values are %f mb %f K" % (P,T)
            for i in range(0, 49):
                cur.execute("select CHANNEL,HV,P0,T0 FROM HVREF WHERE CHANNEL=%d" % (i+1))
                rows = cur.fetchall()
                P0=0
                T0=0
                HV0=0
                for row in rows:
                #    print row
                    HV0=row[1]
                    P0=row[2]
                    T0=row[3]


                #Alice correction
                Vexpected=HV0*P0*T/(P*T0)


                module=i/8
                voie=i%8
                channel=i

                vset=HT.getOutputVoltage(module,voie)
                vmon=abs(HT.getOutputMeasurementSenseVoltage(module,voie))
                imon=HT.getOutputMeasurementCurrent(module,voie)
                status=HT.getOutputStatus(module,voie)

                correction = ((Vexpected/vmon-1)*100)
                if  abs(correction)>1:
                    print "Current voltage is %f and should be %f => correction %f " % (vmon,Vexpected,((Vexpected/vmon-1)*100) )
                    
                else:
                    print "Channel %d is OK" % i

            con.close()
            time.sleep(300)
	

else:
	print "Les modules HT ne sont pas sous tension, fin du programme."


