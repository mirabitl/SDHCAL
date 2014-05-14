sudo yum -y install subversion gcc gcc-c++ libftdi-python libftdi-c++ libftdi-c++-devel libftdi libftdi-devel cmake e2fsprogs-devel mysql-devel  python-devel  qt-devel libxml2-devel libdbi libdbi-devel libdbi-driver sqlite sqlite-devel boost boost-devel scons  root 
cd /data/
svn co  https://lyosvn.in2p3.fr/repository/ilc/src/online online
#xdaq
sudo cp /data/online/add-ons/xdaq.repo /etc/yum.repos.d/
#mettre gpgcheck=0
sudo sed -i -e 's,gpgcheck=1,gpcheck=0,g' /etc/yum.conf
#XDAQ
sudo yum -y groupinstall "extern_coretools"
sudo yum -y groupinstall "coretools"
sudo yum -y groupinstall "extern_powerpack"
sudo yum -y groupinstall "powerpack"
sudo yum -y groupinstall "general_worksuite "
#dim & hal & root
sudo yum -y install daq-dim daq-dim-devel daq-generichal daq-generichal-devel daq-generichal-debuginfo daq-halutilities daq-halutilities-devel daq-halutilities-debuginfo


#mettre dans le ~/.basrc
cat <<! >> ~/.bashrc
if [ -f /data/online/etc/bashrc ]; then
    . /data/online/etc/bashrc
fi
!
. /data/online/etc/bashrc

cd /data/online
 svn co svn://svn.freehep.org/lcio/tags/v01-60 lcio/v01-60 # checkout release v01-60
 cd lcio/v01-60
 mkdir build
 cd build
 cmake ..
 make install
  . /data/online/etc/bashrc 
 #sudo yum -y  install libdbi libdbi-devel libdbi-driver sqlite sqlite-devel
 #sudo yum -y install boost boost-devel scons
 cd /tmp
 tar zxvf /data/online/add-ons/libdbi-drivers-0.8.1a.tar.gz
 cd libdbi-drivers-0.8.1
 ./configure --with-sqlite3 --with-dbi-libdir=/usr/lib64 --with-sqlite3-libdir=/usr/lib64
 make
 sudo make install


cd /data/online/add-ons/
 sudo yum remove openldap-devel
 sudo rpm -Uvh  ILCConfDB-dev-0.22-1.x86_64.rpm ILCConfDB-pythonlib-0.21_sl6-1.x86_64.rpm  ILCConfDB-lib-0.22-1.x86_64.rpm  ILCOracle-lib-11.1-2.x86_64.rpm libxerces-c-3_1-3.1.1-2.1.x86_64.rpm ILCOracle-dev-11.1-1.x86_64.rpm libxerces-c-devel-3.1.1-2.1.x86_64.rpm
 sudo ln -s /lib64/libuuid.so.1.3.0 /lib64/libuuid.so
 sudo mkdir -p /opt/dhcal/lib
 sudo mkdir -p /opt/dhcal/config
sudo chmod -R 777 /opt/dhcal/
 mkdir -p ../opt/dhcal/lib

