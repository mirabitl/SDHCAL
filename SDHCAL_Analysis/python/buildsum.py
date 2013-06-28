#!/usr/bin/python

import sumplot as sp;
from ROOT import *;
import os,sys,time
from ROOT import *

if len(sys.argv) > 1:
    run=int(sys.argv[1])
else:
    print "Please give a run"
    sys.exit(0)
ch=TChain("showers")
fo = open('summary_%d.fit' % run  , 'w+');
r=run;
fo.write("\n| %d|" % r);
sp.loadfile(ch,r);
sp.makeplots(ch,fo);
fo.close()
