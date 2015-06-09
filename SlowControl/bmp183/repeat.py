from bmp183 import bmp183
import os,sys,time
import threading
import MySQLdb as mdb


sql_host='lyosdhcal11'
sql_login='acqilc/RPC_2008'
sql_db='SLOWMAY2015'
if len(sys.argv) > 2:
    sql_host=sys.argv[1]
    sql_login=sys.argv[2]
    sql_db=sys.argv[3] 
else:
    print "Please give SQL Host, login and DB"

bmp = bmp183()
def printit():
  threading.Timer(30.0, printit).start()
  global bmp
  global sql_host,sql_login,sql_db
  sql_name=sql_login.split("/")[0]
  sql_pwd=sql_login.split("/")[1]
  #  print "Hello, World!"
  cnx = mdb.connect(sql_host,sql_name,sql_pwd,sql_db)
  #'lyosdhcal11','acqilc', 'RPC_2008','SLOWAVRIL2015')
  bmp.measure_pressure()
  print time.strftime("%c")," Temperature: ", bmp.temperature, "deg C"," Pressure: ", bmp.pressure/100.0, " hPa"

  cursor = cnx.cursor()


  add_pt = "INSERT INTO PTCOR (P, T) VALUES (%f, %f)" %  (bmp.pressure/100.0,bmp.temperature)

# Insert new employee
  cursor.execute(add_pt)
  emp_no = cursor.lastrowid
  print emp_no
  cnx.commit()

  cursor.close()
  cnx.close()
printit()
