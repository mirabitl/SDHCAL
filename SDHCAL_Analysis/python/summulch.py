
chdict={}
for i in range(0,51):
    chdict[i]=[0.0,0.0,0.0];
    
runlist=[714425,714470,714489,714531,714547,714553,714556,714561,714562,714565]
for i in runlist:
    f=open("chambers_%d.fit" % i)
    lines=f.readlines()
    for l in lines:

        fields=l.split()
        #print fields[0],"..",fields[1],fields[2],fields[3]
        rs=chdict[int(fields[0])]
        rs[0]=rs[0]+float(fields[1])/len(runlist)
        rs[1]=rs[1]+float(fields[2])/len(runlist)
        rs[2]=rs[2]+float(fields[3])/len(runlist)
        chdict[int(fields[0])]=rs
    f.close()

#print chdict
fo=open("chambers_mul.summary","w+")
nch=0;
n0=0.;
n1=0.;
n2=0.
for ich,tab in chdict.iteritems():
    print ich,tab[0],tab[1],tab[2]
    if (tab[0]!=0):
        nch=nch+1
        n0=n0+tab[0]
        n1=n1+tab[1]
        n2=n2+tab[2]
        fo.write("%d %.3f %.3f %.3f \n" % (ich,tab[0],tab[1],tab[2]));
fo.write("%d => %.3f  %.3f %.3f \n" % (nch,n0/nch,n1/nch,n2/nch))
fo.close()
