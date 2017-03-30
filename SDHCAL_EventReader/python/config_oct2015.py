#marlin="./Slot1_39_MARLIN.xml"
marlin="../xml/m3_oct2015.xml"
rebuild=False
useSynch=False
#
minChambersInTime=7;
tkMinPoint=7;
tkExtMinPoint=7;
tkChi2Cut=0.00000000000001;  
tkExtChi2Cut=0.00000000000001;  

tkDistCut=2.;  
tkExtDistCut=4.;

xdaqShift=24;
cerenkovDifId=3
cerenkovOutDifId=3
cerenkovAsicId=1
cerenkovOutAsicId=1
cerenkovOutTimeDelay=6

#
#filePath="/data/NAS/Results/PS_04_2012/DHCAL_%d_I%d_0.slcio"
#filePath="/data/NAS/BeamTest2012Compressed/DHCAL_%d_I%d_%d.slcio"
filePath="/data/NAS/October2015/DHCAL_%d_I%d_%d.slcio"
fileOut="/data/NAS/October2015/STREAMOUT_tmp/DHCAL_%d_SO.slcio"
nevent=0
rootFilePath="./histos/Tracks%d_%d.root"
treeName="./histos/showers_%d_%d.root"
