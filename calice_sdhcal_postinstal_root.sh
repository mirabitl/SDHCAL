#base : SLC cern 5.5
#http: linuxsoft.cern.ch
#rep : /cern/slc55/i386

#install server, cern extra


export PATH=/sbin:/usr/sbin:$PATH
#OS:wq


# post install

# Creation du compte acqilc=20
groupadd acqilc -g 502
adduser acqilc -u 502 -g 502
echo "acqilc  ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Modification de permissions
mkdir /data
chmod 1777 /data
chown -R acqilc:acqilc /home/acqilc
chown -R acqilc:acqilc /data


echo "none  /proc/bus/usb usbfs  defaults,devmode=0666 0 0" >> /etc/fstab
