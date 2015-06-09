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
#		
	for i in range(0, 49):
            module=i/8
            voie=i%8
            print i,HT.getOutputVoltage(module,voie)
		
#	HT.setOutputVoltage(HT.MODULE_0, HT.VOIE_0, 0)
#	HT.setOutputSwitch(HT.MODULE_0, HT.VOIE_0, HT.OFF)
	
	while(1):
            con = mdb.connect('localhost', 'acqilc', 'RPC_2008', 'SLOWAVRIL2015');
            cur=con.cursor();
            for i in range(0, 49):
                module=i/8
                voie=i%8
                channel=i

                vset=HT.getOutputVoltage(module,voie)
                vmon=HT.getOutputMeasurementSenseVoltage(module,voie)
                imon=HT.getOutputMeasurementCurrent(module,voie)
                status=HT.getOutputStatus(module,voie)
                cmd= "INSERT INTO HVMON(CHANNEL,VSET,VMON,IMON,STATUS) VALUES(%d,%f,%f,%f,'%s')" % (channel,vset,vmon,imon,status)
                cur.execute(cmd)
                print i,HT.getOutputVoltage(module,voie),HT.getOutputMeasurementSenseVoltage(module,voie),HT.getOutputMeasurementCurrent(module,voie), HT.getOutputStatus(module,voie)
            con.commit()
            con.close()
            time.sleep(300)
	

else:
	print "Les modules HT ne sont pas sous tension, fin du programme."


