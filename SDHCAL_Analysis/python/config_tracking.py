#marlin="./Slot1_39_MARLIN.xml"
marlin="./can37.xml"
rebuild=False
useSynch=True
#
minChambersInTime=8;
tkMinPoint=8;
tkExtMinPoint=7;
tkChi2Cut=0.00000000000001;  
tkExtChi2Cut=0.00000000000001;  

tkDistCut=9.;  
tkExtDistCut=5.;

#
#filePath="/data/NAS/Results/PS_04_2012/DHCAL_%d_I%d_0.slcio"
#filePath="/data/NAS/Results/DHCAL_%d_I%d_%d.slcio"
filePath="/tmp/DHCAL_%d_I%d_%d.slcio"
nevent=20000
rootFilePath="../Tracks%d_%d.root"
treeName="./showers_%d_%d.root"
