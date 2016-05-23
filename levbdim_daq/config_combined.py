
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
filepath="/data/NAS/February2016"
memorypath="/dev/shm/levbdim"
proclist=["monitor","lcio","basicwriter"]
#proclist=[]
daqhost='lyosdhcal9.in2p3.fr'
daqport=45000
slowhost='lyosdhcal9.in2p3.fr'
slowport=46000
jobhost='lyosdhcal9.in2p3.fr'
jobport=47000
ecalhost='lyoilcrpi58'
ecalport=45000
ecalconfig='/root/calicoes/config/test_rc.xml'
