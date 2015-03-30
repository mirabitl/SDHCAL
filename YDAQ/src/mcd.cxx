#include <yami4-cpp/yami.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "difhw.h"
#include "onedifhandler.h"
#include "evb.h"
#include "browser.h"

int main(int argc, char * argv[])
{
  if (argc != 2)
    {
      std::cout << "expecting three parameters: "
	"server destination \n";
      return EXIT_FAILURE;
    }

  try
    {
      yami::agent client_agent;

      const std::string name_server_address = argv[1];
      Difhw::browser b(name_server_address,&client_agent);
      b.QueryList();
      std::vector<std::string> &vnames=b.getNames();
      std::vector<std::string> &vlocs=b.getLocation();
      std::vector<Difhw::onedifhandler> vds;
      std::size_t size_ = vnames.size();
      vnames.resize(size_);
      vlocs.resize(size_);
      std::string evbname;
      std::string evbloc;
      Evb::Statemachine *evbs=NULL;
      Odb::Statemachine *odbs=NULL;
      for (std::size_t i_ = 0; i_ != size_; ++i_)
        {
	  std::cout<<vnames[i_].substr(0,5)<<"@"<<vlocs[i_]<<std::endl;
	  if (vnames[i_].substr(0,5).compare("#DIF#")==0)
	    {
	      Difhw::onedifhandler odh(vnames[i_],vlocs[i_],&client_agent);
	      vds.push_back(odh);
	    }
	  if (vnames[i_].substr(0,5).compare("#EVB#")==0)
	    {
	      evbs= new Evb::Statemachine(client_agent,vlocs[i_],vnames[i_]);

	      Evb::Config cf;
	      Evb::Status st;
	      cf.Shmpath="/dev/Share";

	      evbs->Initialise(cf,st);
	      
	      std::cout<<st.Evbstatus<<std::endl;
	      getchar();
	    }
	  if (vnames[i_].substr(0,5).compare("#ODB#")==0)
	    {
	      odbs= new Odb::Statemachine(client_agent,vlocs[i_],vnames[i_]);

	      Odb::Config cdb;
	      Odb::Status st;


	      odbs->Initialise(st);
	      cdb.Dbstate="LPCC_230";
	      odbs->Download(cdb,st);
	      std::cout<<st.Oraclestatus<<std::endl;
	      getchar();
	    }
        }



      std::string dummyl;
      std::cin >> dummyl;

      for ( std::vector<Difhw::onedifhandler>::iterator itv=vds.begin();itv!=vds.end();itv++)
	{
	  itv->Scan();
	  itv->Initialise();
	  itv->Print();
	  itv->Configure(0xb000,"UNETATGERE");
	  //itv->Subscribe();
	  itv->Print();
	}
      if (odbs!=NULL)
	{
	  Odb::Status odbstat;
	  odbs->Dispatch(odbstat);
	}
      for ( std::vector<Difhw::onedifhandler>::iterator itv=vds.begin();itv!=vds.end();itv++)
	{
	  itv->LoadSlowControl();
	  //itv->Subscribe();
	  itv->Print();
	}

      if (evbs!=NULL)
	{
	  Evb::Status evbstat;
	  evbs->Start(evbstat);
	}
      std::cin >> dummyl;
      for ( std::vector<Difhw::onedifhandler>::iterator itv=vds.begin();itv!=vds.end();itv++)
	{
	  itv->Start();
	  itv->Print();
	}
      std::cin >> dummyl;
      for ( std::vector<Difhw::onedifhandler>::iterator itv=vds.begin();itv!=vds.end();itv++)
	{
	  itv->Stop();
	  itv->Print();
	}
      std::cin >> dummyl;




	
    }
  catch (const std::exception & e)
    {
      std::cout << "error: " << e.what() << std::endl;
    }
}
