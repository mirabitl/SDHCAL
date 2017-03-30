export SDHCALDIR=${HOME}/SDHCAL
export DHCALDIR=/opt/dhcal
# export DHCALDIR=${HOME}/opt/dhcal

export ROOTSYS=${DHCALDIR}/root
export LCIODIR=${DHCALDIR}/lcio/v02-00
export DIMDIR=${DHCALDIR}/dim
export LEVBDIMDIR=${DHCALDIR}/levbdim

export LD_LIBRARY_PATH=${DHCALDIR}/lib:${DIMDIR}/linux:${LCIODIR}/lib:${ROOTSYS}/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:${DIMDIR}/linux:$LD_LIBRARY_PATH

export PATH=${HOME}/bin:${ROOTSYS}/bin:${DIMDIR}/linux:$PATH

export PYTHONSTARTUP=${SDHCALDIR}/.pythonrc
export PYTHONPATH=${SDHCALDIR}/python:${ROOTSYS}/lib:${PYTHONPATH}



alias rundid='${DIMDIR}/WebDID/webDid &>/dev/null '
alias dhcalinstall='cp ${SDHCALDIR}/opt/dhcal/lib/*.so ${DHCALDIR}/lib/;cp ${SDHCALDIR}/opt/dhcal/bin/* ${DHCALDIR}/bin/'
alias dhnetinstall='cp ${DHCALDIR}/lib/*.so /data/NAS/arm/opt/dhcal/lib/;cp ${DHCALDIR}/bin/* /data/NAS/arm/opt/dhcal/bin/'
alias ladaqavictor='python SDHCAL/python/SDaqImpl.py'

export CONFDB_DEV='ILC/b0T2FB3yxG@(DESCRIPTION = (ADDRESS = (PROTOCOL = TCP)(HOST = ccdbora01.in2p3.fr)(PORT = 1521)) (CONNECT_DATA = (SERVER = DEDICATED) (SERVICE_NAME = ccdev11gtaf.in2p3.fr)))'
export CONFDB_PROD='ILC/fFQeV52rQy@(DESCRIPTION = (ADDRESS = (PROTOCOL = TCP)(HOST = ccdbora01.in2p3.fr)(PORT = 1521)) (CONNECT_DATA = (SERVER = DEDICATED) (SERVICE_NAME = calice.in2p3.fr)))'
export CONFDB_RO='ILC_RO/mK52WBBB29@(DESCRIPTION = (ADDRESS = (PROTOCOL = TCP)(HOST = ccdbora01.in2p3.fr)(PORT = 1521)) (CONNECT_DATA = (SERVER = DEDICATED) (SERVICE_NAME = calice.in2p3.fr)))'
export CONFDB=$CONFDB_PROD
ulimit -s unlimited
