#!/bin/bash
/sbin/lsusb | grep Future | sed 's,Bus \(.*\) Device \(.*\): ID\(.*\),sudo chmod 666 /dev/bus/usb/\1/\2,g' | /bin/sh -
  find /sys -type f -name serial -exec cat {} \; | grep FT101 |  sed -e 's,FT101,,g' > /tmp/ftdi_devices ; cat /tmp/ftdi_devices | wc
