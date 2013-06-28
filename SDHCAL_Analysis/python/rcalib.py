energy={}
fpcut=6
fd9={}
energy[714207]=5.
energy[714425]=15.
energy[714470]=80.
energy[714489]=100.
energy[714531]=80.
energy[714547]=70.
energy[714553]=60.
energy[714556]=50.
energy[714561]=40.
energy[714562]=30.
energy[714565]=20.
energy[715480]=80.
energy[715491]=80.
energy[715493]=70.
energy[715511]=60.
energy[715531]=60.
energy[715551]=50.
energy[715612]=50.
energy[715573]=100.
energy[715592]=90.
energy[715593]=50.
energy[715594]=50.
energy[715595]=50.
energy[715596]=50.
energy[715651]=40.
energy[715671]=30.
energy[715675]=20.
energy[715692]=10.
energy[715693]=10.
energy[715694]=5.
energy[715695]=7.5
energy[715698]=5.
energy[715699]=15.
energy[715700]=25.
energy[715703]=25.
energy[715705]=90.
energy[715706]=100.
energy[715708]=110.
energy[715709]=80.
energy[715710]=110.
#electrons
energy[715713]=40.
energy[715714]=30.
energy[715715]=20.
energy[715716]=50.
energy[715719]=60.
energy[715721]=60.
energy[715723]=70.
energy[715724]=10.
energy[715725]=10.
#pions
energy[715747]=30.
energy[715748]=40.
energy[715751]=50.
energy[715753]=60.
energy[715754]=70.
energy[715756]=80.
energy[715757]=90.
#mu
energy[715763]=30.

npcut={}
ndens={}
fdcut={}
maxhit={}
maxen={}
for r,e in energy.iteritems():
  if (e>=100):
    fd9[r]=0.035
    npcut[r]=17
    ndens[r]=20
    fdcut[r]=0.085
    maxhit[r]=2200.
    maxen[r]=180.
    #print r,e,maxhit[r],maxen[r]
    continue;
  if (e>=90):
    fd9[r]=0.035
    npcut[r]=17
    ndens[r]=20
    fdcut[r]=0.085
    maxhit[r]=2000.
    maxen[r]=160.
    #print r,e,maxhit[r],maxen[r]
    continue;
  if (e>=80):
    fd9[r]=0.04
    npcut[r]=15
    ndens[r]=17
    fdcut[r]=0.085
    maxen[r]=145
    maxhit[r]=1800.
    #print r,e,maxhit[r],maxen[r]
    continue;
  if (e>=70):
    fd9[r]=0.04
    npcut[r]=15
    ndens[r]=15
    fdcut[r]=0.085
    maxen[r]=130
    maxhit[r]=1600.
    #print r,e,maxhit[r],maxen[r]
    continue;
  if (e>=60):
    fd9[r]=0.045
    npcut[r]=15
    ndens[r]=15
    maxen[r]=110.
    maxhit[r]=1400.
    fdcut[r]=0.06
    #print r,e,maxhit[r],maxen[r]
    continue;
  if (e>=50):
    fd9[r]=0.045
    npcut[r]=15
    ndens[r]=13
    ndens[r]=7
    maxen[r]=85.
    maxhit[r]=1200.
    fdcut[r]=0.085
    #print r,e,maxhit[r],maxen[r]

    continue;
  if (e>=40):
    fd9[r]=0.05
    npcut[r]=15
    ndens[r]=10
    maxen[r]=80.
    maxhit[r]=1000.
    fdcut[r]=0.085
    #print r,e,maxhit[r],maxen[r]

    continue;
  if (e>=30):
      fd9[r]=0.05
      npcut[r]=12
      ndens[r]=8
      maxen[r]=60.
      maxhit[r]=1000.
      fdcut[r]=0.085
      #print r,e,maxhit[r],maxen[r]

      continue;
  if (e>=20):
      fd9[r]=0.055
      npcut[r]=12
      ndens[r]=6
      maxen[r]=45.
      maxhit[r]=700.
      fdcut[r]=0.06
      #print r,e,maxhit[r],maxen[r]

      continue;
  if (e>=15):
      fd9[r]=0.06
      npcut[r]=8
      ndens[r]=5
      maxen[r]=30.
      maxhit[r]=500.
      fdcut[r]=0.085
      #print r,e,maxhit[r],maxen[r]
      continue;

  if (e>=10):
      fd9[r]=0.06
      npcut[r]=5
      ndens[r]=3
      maxen[r]=25.
      maxhit[r]=400.
      fdcut[r]=0.085
      #print r,e,maxhit[r],maxen[r]

      continue;
  if (e>=7.5):
      fd9[r]=0.06
      npcut[r]=5
      ndens[r]=3
      fdcut[r]=0.085
      maxen[r]=25.
      maxhit[r]=400.
      #print r,e,maxhit[r],maxen[r]

      continue;
  
  if (e>=5):
      fd9[r]=0.065
      npcut[r]=5
      ndens[r]=3
      fdcut[r]=0.085
      maxen[r]=25.
      maxhit[r]=400
      #print r,e,maxhit[r],maxen[r]

for r,e in energy.iteritems():
  if (r>=714425):
    fdcut[r]=0.085
    npcut[r]=5
    #ndens[r]=3
    fpcut=10
