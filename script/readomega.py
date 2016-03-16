import telnetlib
import time
def readit():
  s=telnetlib.Telnet("lyoilcmeteo01.ipnl.in2p3.fr",1000)
  while True:
    
    p=None
    t=None
    h=None
    d=None
    while (p==None or t==None or h==None or d==None):
      rd=s.read_some()
      rdl=rd.split('\r')
      for x in rdl:
        if len(x)==0:
          continue
        print x,len(x),x[0]
        if x[0]=='P'and len(x)>7:
          p=x[1:6]
        if x[0]=='T'and len(x)>7:
          t=x[1:5]
        if x[0]=='D' and len(x)>5:
          d=x[1:5]
        if x[0]=='H' and len(x)>5:
          h=x[1:5]

      print p,t,h,d
      time.sleep(5)
    
readit()
