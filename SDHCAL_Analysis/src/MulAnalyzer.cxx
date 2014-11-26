#define NX 36
#define NY 36

#include "MulAnalyzer.h"
#include "DIFUnpacker.h"
#include <TLine.h>
#include <TGraphErrors.h>
#include <TFitResult.h>
#include <TFitter.h>
#include <TF1.h>
#include <TPluginManager.h>
#include <stdint.h>
#include <math.h>
#include "TPolyLine3D.h"
#include "TVirtualPad.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
//#include <lapacke.h>
#include "DifGeom.h"




MulAnalyzer::MulAnalyzer() :collectionName_("DHCALRawHits"),theSkip_(0)
{
  reader_=DHCalEventReader::instance();
  rootHandler_ =DCHistogramHandler::instance();
  this->initialise();
}



void MulAnalyzer::initialise()
{
 theImage_.initialise(theImageBuffer_,60,96,96);
}
void MulAnalyzer::initJob(){presetParameters();}
void MulAnalyzer::endJob(){
	
}
void MulAnalyzer::presetParameters()
{
  std::map<std::string,MarlinParameter> m=reader_->getMarlinParameterMap();
  std::map<std::string,MarlinParameter>::iterator it;
  try
    {

      if ((it=m.find("SkipEvents"))!=m.end()) theSkip_=it->second.getIntValue();

      DEBUG_PRINT("Interactif %d \n",draw_);

      //getchar();

    }
  catch (std::string s)
    {
      std::cout<<__PRETTY_FUNCTION__<<" error "<<s<<std::endl;
    }
	
}


void MulAnalyzer::processEvent()
{
  
  if (reader_->getEvent()==0) return;
  evt_=reader_->getEvent();
  //  if (reader_->getRunHeader()!=0)
  //  std::cout<<reader_->getRunHeader()->getParameters().getStringVal("Setup")<<std::endl;
 
  if (evt_->getEventNumber()<=theSkip_) return;
  std::vector<uint32_t> seed;
  IMPL::LCCollectionVec* rhcol=NULL;
  bool rhcoltransient=false,rebuild_=false;
  try {
    rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);
    rebuild_=false;
  }
  catch (...)
    {
      try 
	{
	  evt_->getCollection("RU_XDAQ");
	  rebuild_=true;
	}
      catch (...)
	{
	  printf("No raw data or calo hits \n");
	  exit(0);
	}
    }
  if (rebuild_)
    {
      reader_->parseRawEvent();
      DEBUG_PRINT("End of parseraw \n");
      //reader_->flagSynchronizedFrame();
  
      seed.clear();
      DEBUG_PRINT("Calling CreaetRaw\n");

      rhcol=reader_->createRawCalorimeterHits(seed);
      evt_->addCollection(rhcol,"DHCALRawHits");
      rhcoltransient=false; 


    }
  else
    rhcol=(IMPL::LCCollectionVec*) evt_->getCollection(collectionName_);

  seed.clear();
   
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      uint32_t bc = hit->getTimeStamp();
      if (std::find(seed.begin(), seed.end(), bc)!=seed.end()) continue;
      if (std::find(seed.begin(), seed.end(), bc-1)!=seed.end()) continue;
      if (std::find(seed.begin(), seed.end(), bc+1)!=seed.end()) continue;
      if (std::find(seed.begin(), seed.end(), bc-2)!=seed.end()) continue;
      if (std::find(seed.begin(), seed.end(), bc+2)!=seed.end()) continue;
      seed.push_back(bc);
    }
  printf("Processing %d - %d => nh %d ns %d \n",evt_->getRunNumber(),evt_->getEventNumber(),rhcol->getNumberOfElements(),seed.size());
  for (std::vector<uint32_t>::iterator is=seed.begin();is!=seed.end();is++)
    {
      std::vector<RecoHit*> vrh;
      vrh.clear();
      buildVolume(rhcol,(*is));
      for (uint32_t k=0;k<60;k++)
	{ bool found=false;
	  for (uint32_t i=0;i<96;i++)
	    for (uint32_t j=0;j<96;j++)
	      if (theImage_.getValue(k,i,j)>0) 
		{
		  
		  //printf("%d%d %d %d \n",i,j,hitVolume_[k][i][j].getFlag(RecoHit::EDGE),hitVolume_[k][i][j].getFlag(RecoHit::ISOLATED));
		  //if ((hitVolume_[k][i][j].getFlag(RecoHit::EDGE)==1||hitVolume_[k][i][j].getFlag(RecoHit::ISOLATED)==1) )
		  vrh.push_back(&hitVolume_[k][i][j]);
		  

		}
	}
      
      std::vector<RECOCluster*> clusters;
      clusters.clear();
      for (std::vector<RecoHit*>::iterator ih=vrh.begin();ih<vrh.end();ih++)
	{
	  //printf("Hit plan = %d %d \n",(*ih)->chamber(),(*ih)->plan());
	  //      if ((*ih)->getFlag(RecoHit::CORE)==1) continue;
	  bool merged=false;
	  for (std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
	    {
	      if ((*ih)->chamber()!=(*ic)->chamber()) continue;
	      merged=(*ic)->Append((*(*ih)),2.); // avant 4 et normalement 2
	      if (merged) break;
	    }
	  if (merged) continue;
	  RECOCluster* c= new RECOCluster((*(*ih)));
	  clusters.push_back(c);

	}

      // Now Loop on clusters
      for( std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
	{
	  for (std::vector<RecoHit>::iterator ih=(*ic)->getHits()->begin();ih!=(*ic)->getHits()->end();ih++)
	    {
	      RecoHit& h=(*ih);
	      std::stringstream s0;
	      s0<<"/DIF"<<h.dif()<<"/Asic"<<(int) h.getAsic();
	      TProfile* hmul=(TProfile*) rootHandler_->GetTH1(s0.str()+"/Multiplicity");
	      
	      if (hmul==NULL)
		{
		  printf("Booking %s %d %d \n",s0.str().c_str(),h.dif(),h.getAsic());
		  hmul =rootHandler_->BookProfile(s0.str()+"/Multiplicity",64,0.1,64.1,0.5,5.);
      
		}

	      hmul->Fill(h.getChannel(),(*ic)->size());
	    }
	}
      
    
      for (std::vector<RECOCluster*>::iterator ic=clusters.begin();ic!=clusters.end();ic++)
	delete (*ic);
    }

  if (evt_->getEventNumber()%100 ==0)
    rootHandler_->writeSQL();
}
uint32_t MulAnalyzer::buildVolume(IMPL::LCCollectionVec* rhcol,uint32_t seed)
{
  uint32_t ncount=0;
  theImage_.clear();
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      IMPL::RawCalorimeterHitImpl* hit = (IMPL::RawCalorimeterHitImpl*) rhcol->getElementAt(i);
      uint32_t bc = hit->getTimeStamp();
      if (bc!=seed && bc!=seed-1 && bc!=seed+1 && bc!=seed-2 && bc!=seed+2 ) continue;
      unsigned int difid = hit->getCellID0()&0xFF;
      if (difid<1 || difid>255) continue;
      int asicid = (hit->getCellID0()&0xFF00)>>8;
      int channel= (hit->getCellID0()&0x3F0000)>>16;
      bool thr[3];
      //      DEBUG_PRINT("%x \n",hit->getCellID0());
      int ithr= hit->getAmplitude()&0x3;
      if (ithr==0)
	{
	  std::cout<<difid<<" had:"<<asicid<<":"<<channel<<":"<<ithr<<std::endl;
	  continue;
	}
      std::map<unsigned int,DifGeom>::iterator idg = reader_->getDifMap().find(difid);
      DifGeom& difgeom = idg->second;
      int x=0,y=0;
      uint32_t chid = idg->second.getChamberId();
      uint32_t hrtype=2;
      //if (chid>50) hrtype=11;
      DifGeom::PadConvert(asicid,channel,x,y,hrtype);
      uint32_t I=difgeom.toGlobalX(x);
      if (I<1 || I>96) continue;
      uint32_t J=difgeom.toGlobalY(y);
      if (J<1 || J>96) continue;
      if (chid<1 || chid>60) continue;
		
      //planes.set(chid-1,true);
      theImage_.setValue(chid-1,I-1,J-1,1);

      std::map<unsigned int,ChamberGeom>::iterator icg = reader_->getChamberMap().find( chid);
      ChamberGeom& chgeom = icg->second;
      //printf("Hit beeing filled %d %d %d\n",chid-1,I-1,J-1);
      chgeom.setZ(reader_->getPosition(chid).getZ0());

      hitVolume_[chid-1][I-1][J-1].initialise(difgeom,chgeom,hit,hrtype);

      ncount++;
    }
  DEBUG_PRINT("Total number of Hit in buildVolume %d  %d \n",ncount,seed);
  return ncount;
}


