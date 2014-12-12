import os

def post(run,daqname):
    cmd = '/usr/local/bin/elog -h lyosvn.in2p3.fr -d elog -s -l  "ILC Data Square Meter" -p 443 -u acqilc RPC_2008 -a Auteur="SPS_12_2014" -a Sujet="DataRun"  -a Run="%ld" -a Evenements="started" -x "DAQ Name= %s "' % (run,daqname)
    print cmd
    os.system(cmd)
