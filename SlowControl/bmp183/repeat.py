from bmp183 import bmp183

import threading
import MySQLdb as mdb


bmp = bmp183()
def printit():
  threading.Timer(30.0, printit).start()
  global bmp
  print "Hello, World!"
  cnx = mdb.connect('lyosdhcal11','acqilc', 'RPC_2008',
                              'SLOWAVRIL2015')
  bmp.measure_pressure()
  print "Temperature: ", bmp.temperature, "deg C"
  print "Pressure: ", bmp.pressure/100.0, " hPa"
  cursor = cnx.cursor()


  add_pt = "INSERT INTO PT (P, T) VALUES (%f, %f)" %  (bmp.temperature,bmp.pressure/100.0)

# Insert new employee
  cursor.execute(add_pt)
  emp_no = cursor.lastrowid
  print emp_no
  cnx.commit()

  cursor.close()
  cnx.close()
printit()
