
#include "DimDaqControlServer.h"
#include "DimDaqControl.h"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include "ShmProxy.h"
#include <string.h>



DimDaqControl::DimDaqControl(std::string dns) :theDNS_(dns)  
{
  //DimDaqControlServer* s= new DimDaqControlServer(this);
  /*
  DimClient::setDnsNode(theDNS_.c_str());
  cout<<"Building DimDaqCtrl"<<endl;
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDC/"<<hname<<"/STATUS";
  aliveService_ = new DimService(s0.str().c_str(),processStatus_);
  s0.str(std::string());
  processStatus_=DimDaqControl::ALIVED;
  aliveService_->updateService();
  allocateCommands();
  scandns();
  s0.str(std::string());
  s0<<"DimDaqControl-"<<hname;
  boost::thread wt(worker);
  // DimServer::start(s0.str().c_str()); 
  //  cout<<"Starting DimDaqCtrl"<<endl;
  */

}
DimDaqControl::~DimDaqControl()
{
  /*
  delete aliveService_;
  delete browseCommand_;
  delete scanCommand_;
  delete initialiseCommand_;
  delete registerstateCommand_;
  delete configureCommand_;
  delete startCommand_;
  delete stopCommand_;
  delete destroyCommand_;
*/
  for (std::map<std::string,DimDDSClient*>::iterator it=theDDSCMap_.begin();it!=theDDSCMap_.end();it++)
    delete it->second;
}
/*
void DimDaqControl::allocateCommands()
{
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/DDC/"<<hname<<"/BROWSEDNS";
  browseCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/SCANDEVICES";
  scanCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/INITIALISE";
  initialiseCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/CONFIGURE";
  configureCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/REGISTERSTATE";
  registerstateCommand_=new DimCommand(s0.str().c_str(),"I:1,C",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/START";
  startCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/STOP";
  stopCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  s0.str(std::string());
  s0<<"/DDC/"<<hname<<"/DESTROY";
  destroyCommand_=new DimCommand(s0.str().c_str(),"I:1",this);
  
}
*/
void DimDaqControl::scandns()
{
  // Look for DB server
  DimBrowser* dbr=new DimBrowser(); 
  char *service, *format; 
  int type;
  // Get DB service
  cout<<"On rentre dans scandns "<<endl;
  cout<<"On sort \n";
 char *server,*node;
  dbr->getServers( ); 
  while(dbr->getNextServer(server, node)) 
    { 
      cout << server << " @ " << node << endl; 
    }
  cout<<"0"<<endl;
  dbr->getServices("/DB/*/DOWNLOAD" ); 
  cout<<"1\n";
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/DOWNLOAD");
      cout<<ss.substr(0,n)<<endl;
      theDBPrefix_=ss.substr(0,n);
    } 
  // Get the CCC prefix
  cout<<"2\n";
  dbr->getServices("/DCS/*/STATUS" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/STATUS");
      theCCCPrefix_=ss.substr(0,n);
    } 

  dbr->getServers( ); 
  while(dbr->getNextServer(server, node)) 
    { 
      cout << server << " @ " << node << endl; 
      


      if (strncmp(server,"DimDIFServer",12)!=0) continue;
      std::string ss;
      ss.assign(node);
      size_t n=ss.find(".");
      std::string toto;
      toto=ss.substr(0,n);

      std::stringstream s0;
      s0<<"/DDS/"<<toto;
      DimDDSClient* d=new DimDDSClient(toto,s0.str());
      std::pair<std::string,DimDDSClient*> p(toto,d);
      theDDSCMap_.insert(p);
      
    }
      


}
void DimDaqControl::doScan(DimDDSClient* c)
{
  cout <<"Calling scan \n";
  c->scanDevices();
}
void DimDaqControl::scan()
{
  boost::thread_group g;

  for (std::map<std::string,DimDDSClient*>::iterator it=theDDSCMap_.begin();it!=theDDSCMap_.end();it++)
    {
      g.create_thread(boost::bind(&DimDaqControl::doScan, this,it->second));
    }
  g.join_all();
	
}
void DimDaqControl::doInitialise(DimDDSClient* c)
{
  c->initialise();
}
void DimDaqControl::initialise()
{

  std::stringstream s0;
  s0.str(std::string());
  s0<<theCCCPrefix_<<"/INITIALISE";
  DimClient::sendCommand(s0.str().c_str(),"DCCCCC01"); 
  //  bsem_.lock();
  // theCCCClient_->doInitialise("DCCCCC01");
  s0.str(std::string());
  s0<<theCCCPrefix_<<"/CONFIGURE";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 
  s0.str(std::string());
  s0<<theCCCPrefix_<<"/STOP";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 
  sleep((uint32_t) 1);
  s0.str(std::string());
  s0<<theCCCPrefix_<<"/CCCRESET";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 


  sleep((uint32_t) 1);
  s0.str(std::string());
  s0<<theCCCPrefix_<<"/DIFRESET";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 



  sleep((uint32_t) 1);
  s0.str(std::string());
  s0<<theCCCPrefix_<<"/CCCRESET";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 

  boost::thread_group g;

  for (std::map<std::string,DimDDSClient*>::iterator it=theDDSCMap_.begin();it!=theDDSCMap_.end();it++)
    {
      g.create_thread(boost::bind(&DimDaqControl::doInitialise, this,it->second));
    }
  g.join_all();
	
}
void DimDaqControl::doRegisterstate(DimDDSClient* c)
{
  c->setDBState(theCtrlReg_,theState_);
}
void DimDaqControl::registerstate(uint32_t ctr,std::string sta)
{
  theState_=sta;
  theCtrlReg_=ctr;
  boost::thread_group g;

  for (std::map<std::string,DimDDSClient*>::iterator it=theDDSCMap_.begin();it!=theDDSCMap_.end();it++)
    {
      g.create_thread(boost::bind(&DimDaqControl::doRegisterstate, this,it->second));
    }
  g.join_all();
}

void DimDaqControl::doConfigure(DimDDSClient* c)
{
  c->configure();
}
void DimDaqControl::configure()
{
  std::stringstream s0;

  s0.str(std::string());
  s0<<theCCCPrefix_<<"/CCCRESET";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 


  sleep((uint32_t) 1);
  s0.str(std::string());
  s0<<theCCCPrefix_<<"/DIFRESET";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 



  sleep((uint32_t) 1);
  s0.str(std::string());
  s0<<theCCCPrefix_<<"/CCCRESET";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 
  boost::thread_group g;

  for (std::map<std::string,DimDDSClient*>::iterator it=theDDSCMap_.begin();it!=theDDSCMap_.end();it++)
    {
      g.create_thread(boost::bind(&DimDaqControl::doConfigure, this,it->second));
    }
  g.join_all();
	
}
void DimDaqControl::doStart(DimDDSClient* c)
{
  c->start();
}
void DimDaqControl::start()
{
  boost::thread_group g;

  for (std::map<std::string,DimDDSClient*>::iterator it=theDDSCMap_.begin();it!=theDDSCMap_.end();it++)
    {
      g.create_thread(boost::bind(&DimDaqControl::doStart, this,it->second));
    }
  g.join_all();

  std::stringstream s0;

  s0.str(std::string());
  s0<<theCCCPrefix_<<"/START";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 	
}

void DimDaqControl::doStop(DimDDSClient* c)
{

  c->stop();
}
void DimDaqControl::stop()
{
  std::stringstream s0;

  s0.str(std::string());
  s0<<theCCCPrefix_<<"/STOP";
  DimClient::sendCommand(s0.str().c_str(),(int) 1); 	

  boost::thread_group g;

  for (std::map<std::string,DimDDSClient*>::iterator it=theDDSCMap_.begin();it!=theDDSCMap_.end();it++)
    {
      g.create_thread(boost::bind(&DimDaqControl::doStop, this,it->second));
    }
  g.join_all();
	
}
void DimDaqControl::doDestroy(DimDDSClient* c)
{
  c->destroy();
}
void DimDaqControl::destroy()
{
  boost::thread_group g;

  for (std::map<std::string,DimDDSClient*>::iterator it=theDDSCMap_.begin();it!=theDDSCMap_.end();it++)
    {
      g.create_thread(boost::bind(&DimDaqControl::doDestroy, this,it->second));
    }
  g.join_all();
	
}

/*
void DimDaqControl::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==browseCommand_)
    {
      this->scandns();
      processStatus_=DimDaqControl::BROWSED;
      aliveService_->updateService();
      return ;

    }

  if (currCmd==scanCommand_)
    {
      this->scan();
      processStatus_=DimDaqControl::SCANNED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==initialiseCommand_)
    {
      this->initialise();
      processStatus_=DimDaqControl::INITIALISED;
      aliveService_->updateService();
      return ;

    }

  if (currCmd==registerstateCommand_)
    {
      char* data= (char*) currCmd->getData();
      memcpy(&theCtrlReg_,data,sizeof(uint32_t));
      char* cdata= &data[4];
      theState_.assign(cdata);
      this->registerstate();
      processStatus_=DimDaqControl::DBREGISTERED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==configureCommand_)
    {
      this->configure();
      processStatus_=DimDaqControl::CONFIGURED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==startCommand_)
    {
      this->start();
      processStatus_=DimDaqControl::STARTED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==stopCommand_)
    {
      this->stop();
      processStatus_=DimDaqControl::STOPPED;
      aliveService_->updateService();
      return ;

    }
  if (currCmd==destroyCommand_)
    {
      this->destroy();
      processStatus_=DimDaqControl::DESTROYED;
      aliveService_->updateService();
      return ;

    }
  cout<<"Unknown command \n";
    
  return ;
}
   
*/
