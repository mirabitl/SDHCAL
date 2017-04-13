import accessHisto as ah
from ROOT import *
f=TFile("scurve734771.root")

def calcth(tdc,num):
    h=ah.getth1("/run734771/TDC%d/vth%d" % (tdc,num))
    ax=h.GetXaxis()
    i20=0;i180=0;
    ped=0;width=0;
    for i in range(0,ax.GetNbins()):
        if (h.GetBinContent(ax.GetNbins()-i)>10):
            i20=i;break
    for i in range(0,ax.GetNbins()):
        if (h.GetBinContent(ax.GetNbins()-i)>90):
            i180=i;break
    if (i20>0 and i180>0):
        ped=(ax.GetBinCenter(ax.GetNbins()-i20)+ax.GetBinCenter(ax.GetNbins()-i180))/2
        width=ax.GetBinCenter(ax.GetNbins()-i20)-ax.GetBinCenter(ax.GetNbins()-i180)
        #print ped,width
    else:
        ped=0
        width=0;
        #print num," is dead"
    
    return (ped,width)
def calcall(tdc):
    med={}
    med[1]=322
    med[2]=349
    
    hp=TH1F("hp%d" % tdc,"Pedestal TDC %d" % tdc,32,0.,32.)
    hw=TH1F("hw%d" % tdc,"Width TDC %d" % tdc,32,0.,32.)
    pi=9999;pa=0;
    ped={}
    width={}
    
    for i in range(0,28):
        ped[i]=0
        width[i]=0
        a=calcth(tdc,i)
        if (a[0]>0):
            if (a[0]>pa):
                pa=a[0]
            if (a[0]<pi):
                pi=a[0]
            ped[i]=a[0]
            width[i]=0
        print i,a[0],a[1]
        istrip=0
        ipr=0
        if (i%2==0):
            istrip=i/2
            ipr=istrip
        else:
            istrip=i/2+16
            ipr=31-i/2
        hp.Fill(i+0.1,a[0])
        hw.Fill(i+0.1,a[1])
        
    med[tdc]=(pa+pi)/2
    print "MEDIANE", med[tdc],pa,pi
    dacn={}
    for i in range (0,32):
        dacn[i]=31
    for i in range(0,28):
        istrip=0
        ipr=0
        if (i%2==0):
            istrip=i/2
            ipr=istrip
        else:
            istrip=i/2+16
            ipr=31-i/2
        dac=31
        if (ped[i]>0):
            dac=31+(med[tdc]-ped[i])*0.9/1.46
        #print i,ped[i],width[i],dac
        dacn[ipr]=int(dac)
        if (dacn[ipr]<1):
            dacn[ipr]=1
        if (dacn[ipr]>63):
            dacn[ipr]=63
    st=""
    for i in range (0,32):
        st=st+"%d," % dacn[i]
    print st
    return (hp,hw)
