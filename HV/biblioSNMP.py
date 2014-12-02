#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import subprocess

IP = "128.141.61.46"
#192.168.0.2"

MODULE_0 = 0
MODULE_1 = 1
MODULE_2 = 2
MODULE_3 = 3
MODULE_4 = 4
MODULE_5 = 5
MODULE_6 = 6

VOIE_0 = 0
VOIE_1 = 1
VOIE_2 = 2
VOIE_3 = 3
VOIE_4 = 4
VOIE_5 = 5
VOIE_6 = 6
VOIE_7 = 7

ON = 1
OFF = 0

GROUPSSWITCH_ALL_OFF = 0
GROUPSSWITCH_ALL_ON  = 1
GROUPSSWITCH_RESET_EMERGENCY_OFF = 2
GROUPSSWITCH_SET_EMERGENCY_OFF = 3
GROUPSSWITCH_DISABLE_KILL = 4
GROUPSSWITCH_ENABLE_KILL = 5
GROUPSSWITCH_CLEAR_EVENTS = 10



#-----------------------------------------------------------------------
# getSysMainSwitch()
# snmpget -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.2 sysMainSwitch.0
#-----------------------------------------------------------------------
def getSysMainSwitch():
	base = "snmpget -v 2c -m +WIENER-CRATE-MIB -c public "
	suff = " sysMainSwitch.0"
	commande = base + IP + suff
	#print "### " + commande
	
	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	#print elts[3]
	res = elts[3].split('(')
	#print res
	return res[0]


#-----------------------------------------------------------------------
# Fonction setVoltage(module, voie, tension)
# snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.2 outputVoltage.u0 F val
#-----------------------------------------------------------------------
def setOutputVoltage(module, voie, tension):
	base = "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru "
	pref = " outputVoltage.u"
	suff = " F "
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie) + suff + str(tension)
	#print "### " + commande
	
	stdout = subprocess.check_output(commande, shell=True)
	return stdout
	
	
#-----------------------------------------------------------------------
# Fonction getVoltage(module, voie)	
# snmpget -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.2 outputVoltage.u0
#-----------------------------------------------------------------------
def getOutputVoltage(module, voie):
	base = "snmpget -v 2c -m +WIENER-CRATE-MIB -c public "
	pref = " outputVoltage.u"
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie)
	#print "### " + commande

	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	return float(elts[4])
	

#-----------------------------------------------------------------------
# Fonction setOutputCurrentLimit(module, voie, val)
# snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.2 outputCurrent.u0 F val
#-----------------------------------------------------------------------
def setOutputCurrentLimit(module, voie, val):
	base = "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru "
	pref = " outputCurrent.u"
	suff = " F "
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie) + suff + str(val)
	#print "### " + commande
	
	stdout = subprocess.check_output(commande, shell=True)
	return stdout
	

#-----------------------------------------------------------------------
# Fonction getOutputCurrentLimit(module, voie)	: value in Amps
# snmpget -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.2 outputCurrent.u0
#-----------------------------------------------------------------------
def getOutputCurrentLimit(module, voie):
	base = "snmpget -v 2c -m +WIENER-CRATE-MIB -c public "
	pref = " outputCurrent.u"
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie)
	#print "### " + commande

	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	return float(elts[4])


#-----------------------------------------------------------------------
# Fonction getOutputMeasurementSenseVoltage(module, voie)
# snmpget -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.2 outputMeasurementeSenseVoltage.u0
#-----------------------------------------------------------------------
def getOutputMeasurementSenseVoltage(module, voie):
	base = "snmpget -v 2c -m +WIENER-CRATE-MIB -c public "
	pref = " outputMeasurementSenseVoltage.u"
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie)
	#print "### " + commande

	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	return float(elts[4])


#-----------------------------------------------------------------------
# Fonction getOutputMeasurementCurrent(module, voie)
# snmpget -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.2 outputMeasurementCurrent.u0
#-----------------------------------------------------------------------
def getOutputMeasurementCurrent(module, voie):
	base = "snmpget -v 2c -m +WIENER-CRATE-MIB -c public "
	pref = " outputMeasurementCurrent.u"
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie)
	#print "### " + commande

	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	return float(elts[4])


#-----------------------------------------------------------------------
# Fonction setOutputSwitch(module, voie, valeur) - 1 -> ON / 0 -> OFF
# snmpset -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.2 outputSwitch.u0 i val
#-----------------------------------------------------------------------
def setOutputSwitch(module, voie, val):
	base = "snmpset -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru "
	pref = " outputSwitch.u"
	suff = " i "
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie) + suff + str(val)
	#print "### " + commande

	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	return elts[0]
	
#-----------------------------------------------------------------------
# Fonction getOutputSwitch(module, voie)
# snmpget -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.2 outputSwitch.u0
#-----------------------------------------------------------------------
def getOutputSwitch(module, voie):
	base = "snmpget -v 2c -m +WIENER-CRATE-MIB -c public "
	pref = " outputSwitch.u"
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie)
	#print "### " + commande
	
	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	res = elts[3].split('(')
	return res[0]

#-----------------------------------------------------------------------
# Fonction setOutputVoltageRiseRate(module, voie, val)
# snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.2 outputVoltageRiseRate.u0 F val
#-----------------------------------------------------------------------
def setOutputVoltageRiseRate(module, voie, val):
	base = "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru "
	pref = " outputVoltageRiseRate.u"
	suff = " F "
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie) + suff + str(val)
	#print "### " + commande
	
	stdout = subprocess.check_output(commande, shell=True)
	return stdout


#-----------------------------------------------------------------------
# Fonction getOutputVoltageRiseRate(module, voie)
# snmpget -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.2 outputVoltageRiseRate.u0
#-----------------------------------------------------------------------
def getOutputVoltageRiseRate(module, voie):
	base = "snmpget -v 2c -m +WIENER-CRATE-MIB -c public "
	pref = " outputVoltageRiseRate.u"
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie)
	#print "### " + commande

	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	return float(elts[4])


#-----------------------------------------------------------------------
# Fonction getOutputStatus(module, voie)
# snmpget -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.2 outputStatus.u0
#-----------------------------------------------------------------------
def getOutputStatus(module, voie):
	base = "snmpget -v 2c -m +WIENER-CRATE-MIB -c public "
	pref = " outputStatus.u"
	voie = 100 * module + voie
	commande = base + IP + pref + str(voie)
	#print "### " + commande
	
	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	#print stdout
	#print elts[3]
	#res = elts[3].split('(')
	#print res
	#return res #[0]
	return stdout
	

#-----------------------------------------------------------------------
# Fonction setGroupsSwitch(val)
# snmpset -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.2 groupsSwitch.u0 i val
#-----------------------------------------------------------------------
def setGroupsSwitch(val):
	base = "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru "
	pref = " groupsSwitch.0"
	suff = " i "
	commande = base + IP + pref + suff + str(val)
	#print "### " + commande

	stdout = subprocess.check_output(commande, shell=True)
	elts = stdout.split(' ')
	return elts #[0]
