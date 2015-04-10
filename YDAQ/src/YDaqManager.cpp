#include "YDaqManager.h"
#include <fstream>   
#include "json/json.h"
#include <stdlib.h>

YDaqManager::YDaqManager(std::string nameserver,std::string configuration) : name_server(nameserver), daqconfig(configuration)
{
  this->Parse(daqconfig);
}

void YDaqManager::Parse(std::string conf)
{
  Json::Value root;
  Json::Reader reader;

  std::ifstream ifs (conf.c_str(), std::ifstream::in);

 // Let's parse it  
 
  bool parsedSuccess = reader.parse(ifs, 
                                   root, 
                                   false);
  
  if(not parsedSuccess)
    {
      // Report failures and their locations 
      // in the document.
      std::cout<<"Failed to parse JSON"<<std::endl 
          <<reader.getFormatedErrorMessages()
               <<std::endl;
      return ;
    }
  // HOSTS
  const Json::Value jdifs = root["DIFHOSTS"];
  for ( int index = 0; index < jdifs.size(); ++index )  // Iterates over the sequence elements.
    difhosts.push_back(jdifs[index].asString());
 
  dnshost= root["DNSHOST"].asString();
  evbhost= root["EVBHOST"].asString();
  ccchost= root["CCCHOST"].asString();
  odbhost= root["ODBHOST"].asString();
  zuphost= root["ZUPHOST"].asString();
  // EVB 
  const Json::Value jevb=root["EVB"];
  evbconf.Shmpath=jevb.get("Shmpath","/dev/shm").asString();
  evbconf.Numberoffragment=jevb.get("Numberoffragment",2).asInt();
  evbconf.Publishperiod=jevb.get("Publishperiod",2).asInt();
  evbconf.Outputpath=jevb.get("Outputpath","/tmp").asString();
  evbconf.Outputmode=jevb.get("Outputmode","BINARY").asString();
  evbconf.Monitoringpath=jevb.get("Monitoringpath","NONE").asString();
  evbconf.Monitoringsize=jevb.get("Monitoringsize","10").asInt();
  // CCC
  const Json::Value jccc=root["CCC"];
  cccconf.Serial=jccc.get("Serial","DCCCCC01").asString();
  // ZUP
  const Json::Value jzup=root["ZUP"];
  zupconf.Serial=jzup.get("Serial","/dev/ttyUSB0").asString();
  zupconf.Port=jzup.get("Port","10").asInt();

  // DIF
  const Json::Value jdif=root["DIF"];
  difconf.Dbstate=jdif.get("Dbstate","LPCC_230").asString();
  difconf.Trigger=strtol(jdif.get("Trigger","0x200").asString().c_str(),0,16);

  // ODB
  const Json::Value jodb=root["ODB"];
  odbconf.Dbstate=jodb.get("Dbstate","LPCC_230").asString();

}
void YDaqManager::StopServer()
{
  // DIF
  for (std::vector<std::string>::iterator it=difhosts.begin();it!=difhosts.end();it++)
    {
      std::stringstream s("");
      s<<"ssh pi@"<<(*it)<<" sudo /etc/init.d/yamidifd stop";
      int rc=system(s.str().c_str());
      std::cout<<s.str()<<"===>"<<rc<<std::endl;
    }
  // CCC
  std::stringstream s;
  s<<"ssh pi@"<<ccchost<<" sudo /etc/init.d/yamicccd stop";
  int rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;
  //EVB
  s.str(std::string());
  s<<"ssh acqilc@"<<evbhost<<" sudo /etc/init.d/yamievbd stop";
  rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;
  //ODB
  s.str(std::string());
  s<<"ssh acqilc@"<<odbhost<<" sudo /etc/init.d/yamiodbd stop";
  rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;
  //ZUP
  s.str(std::string());
  s<<"ssh pi@"<<zuphost<<" sudo /etc/init.d/yamizupd stop";
  rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;
  //DNS
  s.str(std::string());
  s<<"ssh acqilc@"<<dnshost<<" sudo /etc/init.d/yamidnsd stop";
  rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;


  
}
void YDaqManager::StartServer()
{
  //DNS
  std::stringstream s;

  s.str(std::string());
  s<<"ssh acqilc@"<<dnshost<<" sudo /etc/init.d/yamidnsd start";
  int rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;

// DIF
  for (std::vector<std::string>::iterator it=difhosts.begin();it!=difhosts.end();it++)
    {
      s.str(std::string());
      s<<"ssh pi@"<<(*it)<<" sudo /etc/init.d/yamidifd start";
      int rc=system(s.str().c_str());
      std::cout<<s.str()<<"===>"<<rc<<std::endl;
    }
  // CCC
  s.str(std::string());

  s<<"ssh pi@"<<ccchost<<" sudo /etc/init.d/yamicccd start";
  rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;
  //EVB
  s.str(std::string());
  s<<"ssh acqilc@"<<evbhost<<" sudo /etc/init.d/yamievbd start";
  rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;
  //ODB
  s.str(std::string());
  s<<"ssh acqilc@"<<odbhost<<" sudo /etc/init.d/yamiodbd start";
  rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;
  //ZUP
  s.str(std::string());
  s<<"ssh pi@"<<zuphost<<" sudo /etc/init.d/yamizupd start";
  rc=system(s.str().c_str());
  std::cout<<s.str()<<"===>"<<rc<<std::endl;

}
// General command
void YDaqManager::Discover()
{
  Difhw::browser b(name_server,&client_agent);
  b.QueryList();
  std::vector<std::string> &vnames=b.getNames();
  std::vector<std::string> &vlocs=b.getLocation();

  std::size_t size_ = vnames.size();
  vnames.resize(size_);
  vlocs.resize(size_);
  difsmap.clear();
  evbs=NULL;
  odbs=NULL;
  cccs=NULL;
  zups=NULL;
  for (std::size_t i_ = 0; i_ != size_; ++i_)
    {
      std::cout<<vnames[i_].substr(0,5)<<"@"<<vlocs[i_]<<std::endl;
      if (vnames[i_].substr(0,5).compare("#DIF#")==0)
	{
	  Difhw::onedifhandler* odh=new Difhw::onedifhandler(vnames[i_],vlocs[i_],&client_agent);
	  std::pair<std::string,Difhw::onedifhandler*> p(vnames[i_],odh);
	  difsmap.insert(p);
	}
      if (vnames[i_].substr(0,5).compare("#EVB#")==0)
	{
	  evbs= new Evb::Statemachine(client_agent,vlocs[i_],vnames[i_]);
	}
      if (vnames[i_].substr(0,5).compare("#ODB#")==0)
	{
	  odbs= new Odb::Statemachine(client_agent,vlocs[i_],vnames[i_]);
	}
      if (vnames[i_].substr(0,5).compare("#CCC#")==0)
	{
	  cccs= new Ccc::Statemachine(client_agent,vlocs[i_],vnames[i_]);
	}
      if (vnames[i_].substr(0,5).compare("#ZUP#")==0)
	{
	  zups= new Zuplv::Statemachine(client_agent,vlocs[i_],vnames[i_]);
	  zups->Initialise(zupconf);
	}
    }
}
void YDaqManager::LVOn()
{
  if (zups!=NULL)
    {
      zups->On(zupstatus);
    }
}
void YDaqManager::LVOff()
{
  if (zups!=NULL)
    {
      zups->Off(zupstatus);
    }
}
void YDaqManager::Initialise()
{
  this->LVOn();

  evbrun.Numberoffragment=0;
  for (std::map<std::string,Difhw::onedifhandler*>::iterator it=difsmap.begin();it!=difsmap.end();it++)
    {
      it->second->Scan();
      it->second->Initialise();
      it->second->Print();
      evbrun.Numberoffragment+=it->second->getStatus().Status.size();
    }

  if (cccs!=NULL)
    {
      cccs->Initialise(cccconf,cccstatus);
      cccs->Configure(cccstatus);
      cccs->Stopacquisition(cccstatus);
      cccs->Cccreset(cccstatus);
      cccs->Difreset(cccstatus);
      cccs->Cccreset(cccstatus);

    }
  if (odbs!=NULL)
    odbs->Initialise(odbstatus);

  if (evbs!=NULL)
    evbs->Initialise(evbconf,evbstatus);
  
}
void YDaqManager::Download()
{
  if (odbs!=NULL)
    odbs->Download(odbconf,odbstatus);
  
  for (std::map<std::string,Difhw::onedifhandler*>::iterator it=difsmap.begin();it!=difsmap.end();it++)
    {
      it->second->Configure(difconf.Trigger,difconf.Dbstate);
      it->second->Print();
    }

  if (odbs!=NULL)
    odbs->Dispatch(odbstatus);
  
}
void YDaqManager::LoadSlowControl()
{
  if (cccs!=NULL)
    {
      cccs->Configure(cccstatus);
    }

  
  for (std::map<std::string,Difhw::onedifhandler*>::iterator it=difsmap.begin();it!=difsmap.end();it++)
    {
      std::cout<<"Loading slow control "<<it->first<<std::endl;
      it->second->LoadSlowControl();
      it->second->Print();
    }
  
}
void YDaqManager::Start()
{
  if (odbs!=NULL)
    odbs->Newrun(odbstatus);

  if (evbs!=NULL)
    {
      evbrun.Run=odbstatus.Run;
      evbrun.Dbstate=difconf.Dbstate;
      evbs->Start(evbrun,evbstatus);
    }
  for (std::map<std::string,Difhw::onedifhandler*>::iterator it=difsmap.begin();it!=difsmap.end();it++)
    {
      it->second->Start();
      it->second->Print();
    }
  if (cccs!=NULL)
    {
      cccs->Start(cccstatus);
    }
}
void YDaqManager::Pause()
{
 if (cccs!=NULL)
    {
      cccs->Stopacquisition(cccstatus);
    }
}
void YDaqManager::Resume()
{
 if (cccs!=NULL)
    {
      cccs->Startacquisitionauto(cccstatus);
    }
}

void YDaqManager::EVBStatus()
{
  if (evbs==NULL)
    {
      std::cout<<"NO EVB "<<std::endl;
    }
   evbs->Currentstatus(evbstatus);
   if (evbstatus.RunValid)
     {
       printf("Run %d started %s #Completed %d \n",evbstatus.Run,evbstatus.Starttime.c_str(),evbstatus.Completed);

       
     }
   if (evbstatus.DifidValid)
     {
       for (int k=0;k<evbstatus.Difid.size();k++)
	 {
	   printf("%4d | %8d | %8d | %12d \n", evbstatus.Difid[k],evbstatus.Gtc[k],evbstatus.Dtc[k],evbstatus.Bcid[k]);
	 }
     }
}

void YDaqManager::DestroyDIF()
{
  for (std::map<std::string,Difhw::onedifhandler*>::iterator it=difsmap.begin();it!=difsmap.end();it++)
    {
      it->second->Destroy();
      it->second->Print();
    }
}
void YDaqManager::Stop()
{
  if (cccs!=NULL)
    {
      cccs->Stop(cccstatus);
    }
  
 for (std::map<std::string,Difhw::onedifhandler*>::iterator it=difsmap.begin();it!=difsmap.end();it++)
    {
      it->second->Stop();
      it->second->Print();
    }
 std::cout<<"DIF Stopped"<<std::endl;
 if (evbs!=NULL)
    {
      evbs->Stop(evbstatus);
    }
}
void YDaqManager::Clear()
{
  for (std::map<std::string,Difhw::onedifhandler*>::iterator it=difsmap.begin();it!=difsmap.end();it++)
    {
      
      delete it->second;
    }
 difsmap.clear();
  evbs=NULL;
  odbs=NULL;
  cccs=NULL;
  zups=NULL;
}
