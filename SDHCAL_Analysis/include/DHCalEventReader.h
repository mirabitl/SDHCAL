#ifndef _DHCALEVENTREADER_H
#define _DHCALEVENTREADER_H
#include "lcio.h"
#include <stdio.h>
#include <bitset>
#include <vector>
#include "IO/LCReader.h"
#include "IMPL/LCTOOLS.h"
#include "IMPL/LCRunHeaderImpl.h" 
#include "IMPL/LCEventImpl.h" 

#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include <IMPL/LCCollectionVec.h>
#include "IMPL/RawCalorimeterHitImpl.h"
#include "IMPL/LCEventImpl.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/LCFlagImpl.h"
// #include "EVENT/SimTrackerHit.h" 
#include <UTIL/LCSplitWriter.h>
#include "UTIL/CellIDDecoder.h"
#include <IO/LCRunListener.h>
#include <IO/LCEventListener.h>

#include <cstdlib>
#include "DCBufferReader.h"

#include "DIFSlowControl.h"
#include "DCType.h"
#include "DHCALAnalyzer.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "FlagTime.h"
#include "DifGeom.h"
#include "ChamberGeom.h"
#include "ChamberPos.h"
#include "PlanShift.h"
#include "MarlinParameter.h"
#include <map>
#include <ext/hash_map>
#ifdef USE_JSON
#include "DHCalJsonParser.h"
#endif
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include "MyInterface.h"
using namespace std ;
/** 
\mainpage



<H1><A NAME="SECTION00010000000000000000">
1 New LCIO format of output data </A>
</H1>

<P>
The data are now stored in LCIO format. Two collections are built
and written:

<UL>
<LI>RU_XDAQ: It is the list of buffer received by the Filter Unit after
the event building. The first buffer is specific to the acquisition,
the next ones are containing the DIF buffer readout after a short
(23 words) XDAQ header.
</LI>
<LI>RawCalorimeterHits: It contains the list of all hits found in the
format described in table <A HREF="#tab:Format-of-the-Raw">1</A>. In addition
an array of six integers containing the DIF counters is added per
DIF as a parameter named DIF_<I>dif-number</I>_Triggers. It contains
DTC,GTC,ATC, TBC in clock unit, and the LBC coded on two integers.
</LI>
</UL>

<BR><P></P>
<DIV ALIGN="CENTER"><A NAME="38"></A>
<TABLE>
<CAPTION><STRONG>Table 1:</STRONG>
Format of the RawCalorimeter hit for
DHCAL</CAPTION>
<TR><TD>
<DIV ALIGN="CENTER">
<TABLE CELLPADDING=3 BORDER="1" ALIGN="CENTER">
<TR><TH ALIGN="CENTER"><B>Field</B></TH>
<TH ALIGN="CENTER"><B>Content</B></TH>
</TR>
<TR><TD ALIGN="CENTER">ID0 bit 0-7</TD>
<TD ALIGN="CENTER">DIF Id</TD>
</TR>
<TR><TD ALIGN="CENTER">ID0 bit 8-15</TD>
<TD ALIGN="CENTER">Asic Id</TD>
</TR>
<TR><TD ALIGN="CENTER">ID0 bit 16-21</TD>
<TD ALIGN="CENTER">Channel</TD>
</TR>
<TR><TD ALIGN="CENTER">ID0 bit 22-31</TD>
<TD ALIGN="CENTER">Module</TD>
</TR>
<TR><TD ALIGN="CENTER">ID1</TD>
<TD ALIGN="CENTER">Bunch crossing time (in clock unit)</TD>
</TR>
<TR><TD ALIGN="CENTER">Amplitude</TD>
<TD ALIGN="CENTER">threshold's bits</TD>
</TR>
<TR><TD ALIGN="CENTER">TimeStamp</TD>
<TD ALIGN="CENTER">distance in clock unit to trigger</TD>
</TR>
</TABLE>
</DIV>
<P>
<DIV ALIGN="CENTER">
</DIV>

<P>

<P>
</TD></TR>
</TABLE>
</DIV><P></P>
<BR>

<P>
The Slow control informations per DIF and Asic are stored in the RunHeader
parameters with the name DIF_<I>#dif_Asic</I>_<I>#asic</I>_<I>parameter</I>.
The parameter names for HardROC2 are described in table yyy.

<P>

<H1><A NAME="SECTION00020000000000000000">
2 DHCAL Analysis code framework</A>
</H1>

<P>
The DHCalEventReader described below, is the main application that
can read or handle DHCAL LCIO events. It first implements basic functionnalities
to open LCIO files, read and write events. It has additional capabilties
to handle and parse Raw data coming from the online. Finally it can
perform analysis. It handles a vector of virtual DHCALAnalyzer 
that are called in each step of the analysis:
initialisation, change of run, new event... Several independant analysis
can be registered and performed simultanously.The histograms handling
is done using ROOT and can be managed with the DCHistogramHandler


*/







/**

\class DHCalEventReader
  \author  L.Mirabito 
  \date March 2010
  \version 1.0

   \brief Main control class of the Digital Hcal analysis 

   <h2> Description</h2>
   The <i>DHCalEventReader </i> is the main class to read/write/analyze events from the DHCAL. It allows to
   <ul>
   <li> Open,read , write LCIO files containing DHCAL events
   <li> Handle and parse raw data from the online and build the corresponding event
   <li> Run any number of analysis sequentially
   
   </ul>

    <h2> Analysis registration</h2>
    The object handles a vector of <i>DHCALAnalyzer*</i>, a virtual analysis class that defines the following methods:
    <ul>
    <li> initJob() 
    <li> endJob() 
    <li> initRun()
    <li> endRun()
    <li> processEvent()
    </ul>
    The DHCalEventReader defines analog methods (initJob,endJonb....analyzeEvent) in which it loops over all registered analysis and calls the
    corresponding methods. A typical implementation of a DHCALAnalyzer would be 
    <pre>
    class MyAnalysis : public DHCALAnalyzer
    {
    public:
         MyAnalysis(DHCalEventReader* rdr) { reader_=r;}
	 void processEvent()  { LCTOOLS::DumpEvent(reader_->getEvent());}
	 void initJob() {;}
....
    private:
        DHCalEventReader* reader_;
     }
     </pre>
     and the main program will be:

     <pre>
     DHCalEventReader a;  // Create the DHCALEventReader
     MyAnalysis ana(&a); // Create an analysis
     a.registerAnalysis(&ana); // Register it
     a.open("Myfile.lcio");  // Open LCIO file
     a.readRun(); // ReadNextRun
     int ier=a.readEvent(); // Read First Event
     while (ier>=0)
     {
       a.buildEvent();  // Build LCIO event if not yet
       a.analyzeEvent();  // call the registered analysis
       ier=a.readEvent(); // read next event
     }
     a.close();
     </pre>

  
 */

class DHCalEventReader : public LCRunListener, public LCEventListener
{
public:
  //! Constructor

  DHCalEventReader();
  //! Destructor 
 
  ~DHCalEventReader();

  // singleton access
  static DHCalEventReader* instance();
 
  //! Get the current event. It does not read it 
  /** 
      @return an IMPL::LCEventImpl ptr
  */
  IMPL::LCEventImpl* getEvent() { return evt_;}
 
  //! Get the current Run Header. It does not read it 
  /** 
      @return an IMPL::LCRunHeaderImpl ptr
  */
  IMPL::LCRunHeaderImpl* getRunHeader(){return runh_;}

  //! Create an empty LCIO event. 
  /** The RU_XDAQ collection is also created and attached to it  
      @param run Run number
      @param det Name of the detector
  */
  void createEvent(int run,std::string det,bool del=true);

  //! Create an empty LCIO Run header  
  /** 
      @param run Run number
      @param det Name of the detector
  */

  void createRunHeader(int run,std::string det);
  
  //! Set the current Event 
  /** 
      @param evt pointer to an IMPL::LCEventImpl
  */
  void setEvent(IMPL::LCEventImpl* evt) {evt_=evt;}
 
  //! Set the current run header 
  /** 
      @param runh pointer to an IMPL::LCRunHeaderImpl
  */
  void setRunHeader(IMPL::LCRunHeaderImpl* runh) { runh_ = runh;}
  
  //! open the given file
  /** 
      @param name file name
  */
  void open(std::string name);

  void open (std::vector< std::string > &filenames);

  //! close the current file
  void close();

  //! Clear internal buffera
  void clear();


  //! Read the stream and call handlers
  /** 
      @return  ier < 0 if end of file
  */
  int readStream(int max_record=0);



  //! Read next event
  /** 
      @return  ier > 0 if new event found
  */
  int readEvent();
int readOneEvent(int run,int event);
  //! Read next run
  void readRun();

  //! Add a RU_XDAQ entry
  /** 
      @param buf int* pointer to the data
      @param rusize size of the buffer in int
  */
  void addRawOnlineRU(int* buf,int rusize);
  
  //! Build the event
  /** 
      @param rebuild false. If True or if they do not exist, the DHCALRawHits are built from the RU_XDAQ collection.
  */
  void buildEvent(bool rebuild=false);

  //! Open the output  file
  /** 
      @param filename file name
  */
  void openOutput(std::string filename);

  //! Write the RunHeader
  void writeRunHeader();


  //! Fill the RunHeader with Slow control informations
  void fillRunHeader();


  //! write the event
  /** 
      @param dropRaw  if true the RU_XDAQ collection is dropped
  */
  void writeEvent(bool dropRaw);

  void write(IMPL::LCEventImpl* evt);

  
  //! close the output file
  void closeOutput();


  //! Register a virtual analysis
  /** 
      @param a  a pointer to a DHCALAnalyzer
  */
  void registerAnalysis(DHCALAnalyzer* a);
  
  //! Loops on all DHCALanalyzer and call processEvent()
  void analyzeEvent();

  //! Loops on all DHCALanalyzer and call initJob()
  void initJob();

  //! Loops on all DHCALanalyzer and call endJob()
  void endJob();

  //! Loops on all DHCALanalyzer and call initRun()
  void initRun();

  //! Loops on all DHCALanalyzer and call endRun()
  void endRun();

  //! return the data format version 
  /**
     @return  version number read from the first DIF buffer
  */
  unsigned int getVersion() { return version_;}

  
  //! return the hardroc type 
  /**
     @return hardroc DIF type read from the first DIF buffer
  */
  uint32_t getHardRocType(){ return hrType_;}


  //! Test of SLowControl read
  /** 
      @return True if the last DIF read contains slow control data 
  */
  bool isSlowControl(){ return (nslow_!=0);}

  //! Access to the analysis vector
  /** 
      @return  vector of ptr to DHCALAnalyzer
  */
  std::vector<DHCALAnalyzer*> getAnalysis(){ return vProcess_;}


  //! Interface for readStream
  void processEvent( LCEvent * evt )  ;
  //! Interface for readStream
  void processRunHeader( LCRunHeader* run) ;
  //! Interface for readStream
  void modifyEvent( LCEvent * evt ) { /* not needed */ ;}
  //! Interface for readStream
  void modifyRunHeader(LCRunHeader* run){ /* not needed */ ;}
  //! <i>Internal</i> parse the DIF buffer 
  int parseRawEvent(int rushift=23);
  //! register dif chambers
  void difInChamber(uint32_t dif,uint32_t ch);
  //! Get Chamber of a dif
  uint32_t getChamber(uint32_t dif);

  //! compactify
  void compactify(uint32_t synchcut=0,uint32_t nfcut=3); 

  //! Flag frames in synch in different chambers (+-1)
  void flagSynchronizedFrame(uint32_t synchcut=0,uint32_t nfcut=3);


   void findTimeSeeds(  int32_t nhit_min,std::vector<uint32_t>& candidate);
  void findTimeSeeds(  int32_t nasic_min);
  void findDIFSeeds(  int32_t nasic_min);
  //! <i>Internal</i> create the DHCALRawHits collection 
  IMPL::LCCollectionVec* createRawCalorimeterHits(bool usesynch=false);
  IMPL::LCCollectionVec* createRawCalorimeterHits(std::vector<uint32_t> seeds);

  int DebugBuffer(unsigned char* buf,uint32_t max_size);
  void setDebug(uint32_t l){debugLevel_=l;}
  void ParseSteering(std::string filename);
  void ParseElement(xmlNode * a_node);
  void fastFlag(uint32_t seedCut=2,uint32_t minChamberInTime=3);
  void fastFlag2(std::vector<uint32_t> &seed,uint32_t seedCut=2,uint32_t minChamberInTime=5);
  std::map<uint32_t,DifGeom>& getDifMap(){return geodifmap_;}
  std::map<uint32_t,ChamberGeom>& getChamberMap(){return geochambermap_;}
  std::map<uint32_t,ChamberPos>& getPositionMap(){return poschambermap_;}
  ChamberPos& getPosition(uint32_t id){ return poschambermap_[id];} 
  void setDropFirstRU(bool t){dropFirstRU_=t;}
  std::map<std::string,MarlinParameter>& getMarlinParameterMap(){return  theMarlinParameterMap_;}
  void setXdaqShift(unsigned int s){theXdaqShift_=s;}
  void setCerenkovDifId(int id){m_cerenkovDifId=id;}
  void setCerenkovOutDifId(int id){m_cerenkovOutDifId=id;}
  void setCerenkovOutAsicId(int id){m_cerenkovOutAsicId=id;}
  void setCerenkovOutTimeDelay(int td){m_cerenkovOutTimeDelay=td;}


  //void addFile(std::string s) {filenames_.push_back(s);}

  void addFile(std::string s) {filenames_.push_back(s);}
  uint32_t getNumberOfEvents(){return lcReader_->getNumberOfEvents();}
  void openFiles(){this->open(filenames_);}
  void findEvent(int run,int event);
float getBeamEnergy() {return BeamEnergy_;}
#ifdef USE_JSON
  DHCalJsonParser* getDHCalJsonParser(){return &theJsonParser_;}
  void parseJsonConfig(std::string cfg){theJsonParser_.parse(cfg);}
#endif
  std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >& getPhysicsEventMap(){return thePhysicsEventMap_;}
  std::vector<uint32_t>& getTimeSeeds(){return theTimeSeeds_;}
  std::vector<uint32_t>& getDIFSeeds(){return theDIFSeeds_;}
  std::vector<DIFPtr*>& getDIFList(){return  theDIFPtrList_;}
  void correctGeometry();
  void stopReadMemory();
  void startReadMemory(std::string directory,uint32_t nd,uint32_t run);
  void startReadFile(std::string name);
  void serviceReadMemory();
  void serviceReadFile();


  void readGeometry(std::string account,std::string testname);
  void dumpGeometry();
void queryCutFiles(std::string cut);
  void queryEnergyFiles(float energy,bool compress=true);
  void queryFiles(uint32_t run,bool compress=true);
  void processQueriedFiles(uint32_t nev);
  void logbookBeamEnergy(uint32_t run);
 private:
  LCReader* lcReader_; /// LCIO Reader
  //  LCSplitWriter* lcWriter_; /// LCIO Writer
  LCWriter* lcWriter_; /// LCIO Writer
  IMPL::LCEventImpl* evt_; /// LCIO Event ptr
  IMPL::LCEventImpl* evtOutput_; /// LCIO Event output ptr
  IMPL::LCRunHeaderImpl* runh_; /// LCIO Run Heder ptr
 

#ifdef USE_JSON
  DHCalJsonParser theJsonParser_;
#endif

  std::vector<DCDIF*> vdif_; /// <i>Internal </i> to handle DIF
  std::vector<DCFrame*> vframe_; /// <i>Internal </i> to handle Frame
  std::vector<DIFSlowControl* > vslow_; /// <i>Internal </i> to handle SlowControl

  std::vector<std::string> filenames_;
  std::vector<LCReader*> readers_;
  std::vector<DHCALAnalyzer*> vProcess_; /// Vector of DHCALAnalyzer
  
  uint32_t version_; /// version of data
  uint32_t hrType_; /// Hardroc type
  uint32_t nslow_; /// Number of DIF Slow control buffers read
  bool newRunHeader_;/// True if new run header found
  std::map<uint32_t,uint32_t> chambers_;

  bool compactify_;
  uint32_t bd_synchcut_,bd_nfcut_,debugLevel_;
  uint32_t nGood_,nBad_,nBadTwo_,nZero_,expectedDtc_;
  DCDIF* difPtr0_;
  DCDIF* difPtr_;
  DCFrame* framePtr0_;
  DCFrame* framePtr_;
  FlagTime flagger_;

  std::map<uint32_t,DifGeom> geodifmap_; 
  std::map<uint32_t,ChamberGeom> geochambermap_; 
  std::map<uint32_t,ChamberPos> poschambermap_; 
  std::map<uint32_t,PlanShift> planshiftmap_; 
  std::map<std::string,MarlinParameter>  theMarlinParameterMap_;
  bool dropFirstRU_;
  uint32_t theXdaqShift_;
  
  std::vector<DIFPtr*> theDIFPtrList_;

  std::string currentFileName_;
  static DHCalEventReader* _me;
  std::vector<uint32_t> theTimeSeeds_;
  std::vector<uint32_t> theDIFSeeds_;
  
  std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> > thePhysicsEventMap_;

  bool monitoringStart_;
  std::string monitoringDirectory_;
  uint32_t monitoringNDIF_,monitoringRun_;
  boost::thread    monitoringThread_;
  MyInterface* my_;
  uint32_t versionid_;
  float BeamEnergy_;
  std::vector<std::string> vFiles_;
};

#endif
