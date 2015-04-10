export ROOTSYS=/opt/dhcal//root

export LCIODIR=/opt/dhcal/lcio/v02-00/
export SDHCALDIR=${HOME}/SDHCAL
export DIMDIR=//opt/dhcal/dim
export LD_LIBRARY_PATH=/opt/dhcal/lib:${DIMDIR}/linux:${LCIODIR}/lib:${ROOTSYS}/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:${DIMDIR}/linux:$LD_LIBRARY_PATH

export PATH=${HOME}/bin:${ROOTSYS}/bin:${DIMDIR}/linux:$PATH

export PYTHONSTARTUP=${SDHCALDIR}/.pythonrc
export PYTHONPATH=$HOME/SDHCAL/python:${ROOTSYS}/lib:${PYTHONPATH}



alias rundid='${DIMDIR}/WebDID/webDid &>/dev/null '
alias dhcalinstall='cp $SDHCALDIR/opt/dhcal/lib/*.so /opt/dhcal/lib/;cp $SDHCALDIR/opt/dhcal/bin/* /opt/dhcal/bin/'
alias dhnetinstall='cp /opt/dhcal/lib/*.so /data/NAS/arm/opt/dhcal/lib/;cp /opt/dhcal/bin/* /data/NAS/arm/opt/dhcal/bin/'
alias ladaqavictor='python SDHCAL/python/SDaqImpl.py'

export CONFDB_DEV='ILC/b0T2FB3yxG@(DESCRIPTION = (ADDRESS = (PROTOCOL = TCP)(HOST = ccdbora01.in2p3.fr)(PORT = 1521)) (CONNECT_DATA = (SERVER = DEDICATED) (SERVICE_NAME = ccdev11gtaf.in2p3.fr)))'
export CONFDB_PROD='ILC/fFQeV52rQy@(DESCRIPTION = (ADDRESS = (PROTOCOL = TCP)(HOST = ccdbora01.in2p3.fr)(PORT = 1521)) (CONNECT_DATA = (SERVER = DEDICATED) (SERVICE_NAME = calice.in2p3.fr)))'
export CONFDB_RO='ILC_RO/mK52WBBB29@(DESCRIPTION = (ADDRESS = (PROTOCOL = TCP)(HOST = ccdbora01.in2p3.fr)(PORT = 1521)) (CONNECT_DATA = (SERVER = DEDICATED) (SERVICE_NAME = calice.in2p3.fr)))'
export CONFDB=$CONFDB_PROD
ulimit -s unlimited
