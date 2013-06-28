#!/usr/bin/python
import LSDHCALReadout
import socket
import time
sdc=LSDHCALReadout.DIFServer(socket.gethostname(),5000)
while (True):
  time.sleep(1)
