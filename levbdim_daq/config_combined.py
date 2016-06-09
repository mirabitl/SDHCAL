
jsonfile='/data/NAS/config/levbdim_m3_good.json'

zupdevice='/dev/ttyUSB0'
zupport=1
dccname="DCCCCC01"
mdccname="MDCC01"

ctrlreg=0x89580000
# 115 + petit seup
dbstate="Dome_42chambres_Reference_v4_163"
# Novermber 2012 state="Dome_42chambres_Reference_v4_115"
# November 2012 + masks state="Dome_42chambres_Reference_v4_144"
filepath="/data/NAS/June2016"
memorypath="/dev/shm/levbdim"
proclist=["monitor","root","basicwriter"]
#proclist=[]
daqhost='lyosdhcal9.cern.ch'
daqport=45000
slowhost='lyosdhcal9.cern.ch'
slowport=46000
jobhost='lyosdhcal9.cern.ch'
jobport=47000
ecalhost='llrcaldaq'
ecalport=45000
ecalconfig='/opt/calicoes/config/combined.xml'
#ecaldetid=1100
#ecalsourceid=[1,2,3,4,5,6,7,8,9,10]
