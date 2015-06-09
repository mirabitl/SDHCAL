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

if (HT.getSysMainSwitch() == "on"):
	print "Vérification de l'alimentation... OK !"
	
	print "Initialisation des modules"
	for i in range(0, 8):
		HT.setOutputVoltage(HT.MODULE_0, i, 0)
		
	for i in range(0, 8):
		print HT.getOutputVoltage(HT.MODULE_0, i)
		
	HT.setOutputVoltage(HT.MODULE_0, HT.VOIE_0, 0)
	HT.setOutputSwitch(HT.MODULE_0, HT.VOIE_0, HT.OFF)
	
	while(1):
		print HT.getOutputStatus(HT.MODULE_0, HT.VOIE_0)
		print HT.getOutputMeasurementSenseVoltage(HT.MODULE_0, HT.VOIE_0)
		print HT.getOutputMeasurementCurrent(HT.MODULE_0, HT.VOIE_0)
		time.sleep(1)
	

else:
	print "Les modules HT ne sont pas sous tension, fin du programme."


