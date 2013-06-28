#include "lcio.h"
#include <stdio.h>
#include <bitset>
#include <vector>
#include "IO/LCReader.h"
#include "IMPL/LCTOOLS.h"
#include "IMPL/LCRunHeaderImpl.h" 
#include "IMPL/LCEventImpl.h" 

#include "EVENT/SimCalorimeterHit.h" 
#include "EVENT/CalorimeterHit.h" 
#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include <IMPL/LCCollectionVec.h>
#include "IMPL/RawCalorimeterHitImpl.h"
#include "IMPL/LCEventImpl.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/LCFlagImpl.h"
// #include "EVENT/SimTrackerHit.h" 

#include "UTIL/CellIDDecoder.h"

#include <cstdlib>
#include "DCBufferReader.h"
#include "DIFSlowControl.h"
#include "DCType.h"
#include "RKAnalyzerTB_Oct.h"
using namespace std ;
using namespace lcio ;
/** dump the given event to screen
 */
#include "GenericOnlineReader.h"

///////////////////////////
//Options to run the code//
///////////////////////////
#define STEP_BY_STEP

///////////////////////////
//////////MAIN/////////////
///////////////////////////
int main(int argc, char** argv ){


  char* FILEN ;
  char* FILEO;
  int runNumber=0 ;
  int evtNumber=0 ;
  int nthEvent=1 ;

	// read file name from command line (only argument) 
	if( argc < 2 ){

		cout << " usage: dumpevent filename runNum evtNum " << endl ;
		cout << "    or: dumpevent filename n      " << endl ;
		cout << "  where the first dumps the event with the specified run and event number" << endl ;
		cout << "  and the second simply dumps the n-th event in the file" << endl ;
		exit(1) ;
	}
  
	FILEN = argv[1] ;

	bool writing=(argc==3);
	if (writing)
		FILEO= argv[2];
//////////////////////////////////////////////////////
//////PREMIERE TECHNIQUE POUR LIRE LES DATA SLCIO/////
//////////////////////////////////////////////////////
	cout<<"Technique de dump 1"<<endl;
	GenericOnlineReader dher;
    RKAnalyzerTB_Oct *a= new RKAnalyzerTB_Oct(&dher);
	(*a).initJob();
    dher.registerAnalysis(a);

//////////////////////////////////////////////////////////	
///Premier passage pour allignment////////////////////////	
//////////////////////////////////////////////////////////	
	
//Récupère la file ./LCIO	
	std::string name(FILEN);
	dher.open(name);
	
	if (writing){
		std::string nameo(FILEO);
		dher.openOutput(FILEO);
	}
	int nevt=0;
	//Pour sortir tout les params du RUN
	dher.readRun(); //Nécessaire pour setter des truc aussi sinon ca tourne pas....
	//Pour Dumper le "next event" (commence par le premier) 
	//(a chaque fois qu'on utilise la fonction on avance d'un et à la fin quand il y'en à plus ca renvoi une exeption)
	//Ici on utilise la fonction LCTOOLS::dumpEvent qui est appelée par DHCalEventReader::readEvent()
	int ier = dher.readEvent();  
	nevt++;//Pour compter combien en en à lu!

	 //LCTOOLS::dumpEvent( dher.getEvent() ) ;
#ifdef STEP_BY_STEP
	//getchar();
#endif 
    //Traitement du premier évenement
	if (ier>0)
	{
		std::cout<<"______________________________________________________________________"<<endl;
		cout<<"Event no: "<<nevt<<endl;
		//dher.parseRawEvent();
		dher.buildEvent();
		dher.analyzeEvent();
		if (writing)
		{
			//	  dher.buildEvent();
			dher.writeRunHeader();
			dher.writeEvent(true);
		}
	}

	
	//Traitement des évenements suivants
	while ((ier>0)&&(nevt<1000))
    {
		//if(nevt>130)break; //To make tests
		//getchar();
		std::cout<<"______________________________________________________________________"<<endl;
		cout<<"Event no: "<<nevt+1<<endl;
		ier = dher.readEvent();
		try {
			dher.buildEvent();
			//dher.parseRawEvent();
		}
		catch (std::string e )
		{
			std::cout<<e<<std::endl;
			break;
		}
		nevt++;
		if (ier<=0) break;
		
		if (writing)
		{
			dher.buildEvent();
			//dher.parseRawEvent();
			dher.writeEvent(true);
		}
		else{
			dher.analyzeEvent();
		}
		//getchar();
	}
	
	dher.close();
//FIN du premier tour d'alignement	
	
	(*a).FirstAlign();
	(*a).SetStatus(2); 
	
	
	
	dher.open(name);
	//(*a).PrintEfficiencyResults();

	if (writing){
		std::string nameo(FILEO);
		dher.openOutput(FILEO);
	}
	nevt=0;
	//Pour sortir tout les params du RUN
	dher.readRun(); //Nécessaire pour setter des truc aussi sinon ca tourne pas....
	//Pour Dumper le "next event" (commence par le premier) 
	//(a chaque fois qu'on utilise la fonction on avance d'un et à la fin quand il y'en à plus ca renvoi une exeption)
	//Ici on utilise la fonction LCTOOLS::dumpEvent qui est appelée par DHCalEventReader::readEvent()
	ier = dher.readEvent();  
	nevt++;//Pour compter combien en en à lu!
	
	//LCTOOLS::dumpEvent( dher.getEvent() ) ;
#ifdef STEP_BY_STEP
	//getchar();
#endif 
    //Traitement du premier évenement
	if (ier>0)
	{
		std::cout<<"______________________________________________________________________"<<endl;
		cout<<"Event no: "<<nevt<<endl;
		//dher.parseRawEvent();
		dher.buildEvent();
		dher.analyzeEvent();
		if (writing)
		{
			//	  dher.buildEvent();
			dher.writeRunHeader();
			dher.writeEvent(true);
		}
	}
	
	
	//Traitement des évenements suivants
	while ((ier>0)&&(nevt<1000))
    {
		//if(nevt>130)break; //To make tests
		//getchar();
		std::cout<<"______________________________________________________________________"<<endl;
		cout<<"Event no: "<<nevt+1<<endl;
		ier = dher.readEvent();
		try {
			dher.buildEvent();
			//dher.parseRawEvent();
		}
		catch (std::string e )
		{
			std::cout<<e<<std::endl;
			break;
		}
		nevt++;
		if (ier<=0) break;
		
		if (writing)
		{
			dher.buildEvent();
			dher.writeEvent(true);
		}
		else{
			dher.analyzeEvent();
		}
	}
	
	
	
	
	if (writing) 
		dher.closeOutput();
	else
		dher.writeHistograms("Beams.root");
	
	dher.close();
	
	
	
	
	
	std::cout<<"FINI"<<std::endl;

	if (1>0) return 0;
	
	
}
