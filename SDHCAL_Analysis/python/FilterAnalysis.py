#!/usr/bin/env python
import DHCalEventReader as dr
import os,sys
import getopt

def usage():
    print __doc__
def main(argv):
    mod_name="config_filter"
    seq_min=0
    seq_max=0
    run=0 

    try:                                
        opts, args = getopt.getopt(argv, "hC:S:s:r:", ["help","mod_name=","seq_max=","seq_min=","run="])
    except getopt.GetoptError:
        print "Error"
        usage()                         
        sys.exit(2)

    print opts
    for opt, arg in opts:
        print opt, arg
        if opt in ("-h", "--help"): 
            usage()
            sys.exit()                  
        elif opt in ("-C", "--config-file"): 
            mod_name = arg               
        elif opt in ("-S", "--max-sequence"): 
            seq_max = int(arg)               
        elif opt in ("-s", "--min-sequence"): 
            seq_max = int(arg)               
        elif opt in ("-r", "--run"):
            print "Run option"
            run = int(arg)   

            print run
                
    if (run==0):
        print "Please give a run Number"
        usage()
        sys.exit()


    try:
        exec("import %s  as config" % mod_name)
    except ImportError:
        raise Exception("cannot import")



    fileList=[]

    for iseq in range(seq_min,seq_max+1):
        fname = config.InputFileDir+ config.InputFilePattern % (run,config.Instance,iseq)
        print fname," will be read"
        fileList.append(fname)

#fileList=["/data/online/Results/DHCAL_22065_I0_0.slcio"]


    fileOut= config.OutputFilePattern % run
    #fileOut="/tmp/t27ch.slcio"
  
    dher=dr.DHCalEventReader();
    rootHandler=dr.DCHistogramHandler()






    dher.ParseSteering(config.Marlin)
        
    a=dr.FilterAnalyzer( dher,rootHandler);

    a.setuseSynchronized(config.useSynchronized);
    a.setminChambersInTime(config.minChambersInTime);
    a.setWriting(True)

    dher.openOutput(fileOut)
    dher.registerAnalysis(a);
    for x in fileList:
        print "================================>",x
        dher.open(x)
        dher.readStream()
        dher.close()  

    dher.closeOutput()
#rootHandler.writeHistograms("toto20650.root")
if __name__ == "__main__":
    main(sys.argv[1:])
