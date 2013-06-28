#!/usr/bin/python
import os
import sys
import listrun2012
if len(sys.argv) > 1:
    seq=int(sys.argv[1])
else:
    print "Please give a sequence file name"


for run in listrun2012.run05:
    os.system("./ChamberAnalysis.py %d config_tracking %d " % (run,seq))
    os.system("mv ./showers_%d_%d.root /data/NAS/Ntuple/" % (run,seq))
