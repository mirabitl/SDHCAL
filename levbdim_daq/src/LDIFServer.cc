
#include "LDIFServer.hh"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include "ftdi.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <string.h>

using namespace Ftdi;
void LDIFServer::registerdb(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::string dbsta=m->content()["dbstate"].asString();
   this->registerDB(dbsta);

}


void LDIFServer::scan(levbdim::fsmmessage* m) 
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

   this->prepareDevices();
   std::map<uint32_t,FtdiDeviceInfo*>& fm=this->getFtdiMap();
   std::map<uint32_t,LDIF*> dm=this->getDIFMap();
   //LOG4CXX_INFO(_logLdaq," CMD: SCANDEVICE clear Maps");
   for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
     { if (it->second!=NULL) delete it->second;}
   dm.clear();
   // _ndif=0;
   Json::Value array;
   for ( std::map<uint32_t,FtdiDeviceInfo*>::iterator it=fm.begin();it!=fm.end();it++)
     {

       LDIF* d= new LDIF(it->second);
       this->getDIFMap().insert(std::make_pair(it->first,d));
       LOG4CXX_INFO(_logLdaq," CMD: SCANDEVICE created LDIF @ "<<std::hex<<d<<std::dec);
       Json::Value jd;
       jd["detid"]=d->detectorId();
       jd["sourceid"]=it->first;
       array.append(jd);
     }

   m->setAnswer(array);
}

void LDIFServer::initialise(levbdim::fsmmessage* m)
{
  
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}
      
      itd->second->initialise();
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  LOG4CXX_INFO(_logLdaq," calling initialise LDIF @ "<<std::hex<<it->second<<std::dec);
	  it->second->initialise();
	}
      return;
    }
}


void LDIFServer::setThreshold(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t difid=atoi(request.get("difid","0").c_str());
  uint32_t B0=atoi(request.get("B0","0").c_str());
  uint32_t B1=atoi(request.get("B1","0").c_str());
  uint32_t B2=atoi(request.get("B2","0").c_str());
  int32_t ctrlreg1=atoi(request.get("CTRLREG","0").c_str());

  Json::Value jt;
  jt["ctrlreg"]=request.get("CTRLREG","0");
  uint32_t ctrlreg= jt["ctrlreg"].asUInt();
  LOG4CXX_INFO(_logLdaq," Threshold changed with "<<difid<<" ctr "<<ctrlreg<<" B0 "<<B0<<" B1 "<<B1<<" B2 "<<B2<<" et "<<request.get("CTRLREG","0").c_str()<<" "<<jt<<" "<<ctrlreg1);

  if (B0==0 || B1==0 || B2==0|| ctrlreg==0)
    {
      LOG4CXX_ERROR(_logLdaq," Invalid parameters dif "<<difid<<" ctr "<<ctrlreg<<" B0 "<<B0<<" B1 "<<B1<<" B2 "<<B2);
      response["STATUS"]="Invalid params ";
      return;
    }
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  Json::Value array_slc;
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");

	  response["STATUS"]="DIFID not found ";
	  return;

	}
      itd->second->setThreshold(B0,B1,B2);
      itd->second->configure(ctrlreg);
      Json::Value ds;
      ds["id"]=itd->first;
      ds["slc"]=itd->second->status()->slc;
      array_slc.append(ds);
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->setThreshold(B0,B1,B2);
	  it->second->configure(ctrlreg);
	  Json::Value ds;
	  ds["id"]=it->first;
	  ds["slc"]=it->second->status()->slc;
	  array_slc.append(ds);


	}
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;
      return;
    }
}



void LDIFServer::cmdStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t difid=atoi(request.get("difid","0").c_str());
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  Json::Value array_slc;
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  response["STATUS"]="DIFID not found ";
	  return;
	}
      

      Json::Value ds;
      ds["detid"]=itd->second->detectorId();
      ds["state"]=itd->second->state();
      ds["id"]=itd->second->status()->id;
      ds["status"]=itd->second->status()->status;
      ds["slc"]=itd->second->status()->slc;
      ds["gtc"]=itd->second->status()->gtc;
      ds["bcid"]=(Json::Value::UInt64)itd->second->status()->bcid;
      ds["bytes"]=(Json::Value::UInt64)itd->second->status()->bytes;
      ds["host"]=itd->second->status()->host;
      array_slc.append(ds);
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{

	  Json::Value ds;
	  ds["detid"]=it->second->detectorId();
	  ds["state"]=it->second->state();
	  ds["id"]=it->second->status()->id;
	  ds["status"]=it->second->status()->status;
	  ds["slc"]=it->second->status()->slc;
	  ds["gtc"]=it->second->status()->gtc;
	  ds["bcid"]=(Json::Value::UInt64) it->second->status()->bcid;
	  ds["bytes"]=(Json::Value::UInt64)it->second->status()->bytes;
	  ds["host"]=it->second->status()->host;
	  array_slc.append(ds);



	}
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;


      return;
    }
}

void LDIFServer::status(levbdim::fsmmessage* m)
{

  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();

  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  Json::Value array_slc;
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}
      

      Json::Value ds;
      ds["detid"]=itd->second->detectorId();
      ds["state"]=itd->second->state();
      ds["id"]=itd->second->status()->id;
      ds["status"]=itd->second->status()->status;
      ds["slc"]=itd->second->status()->slc;
      ds["gtc"]=itd->second->status()->gtc;
      ds["bcid"]=(Json::Value::UInt64)itd->second->status()->bcid;
      ds["bytes"]=(Json::Value::UInt64)itd->second->status()->bytes;
      ds["host"]=itd->second->status()->host;
      array_slc.append(ds);
      m->setAnswer(array_slc);
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{

	  Json::Value ds;
	  ds["detid"]=it->second->detectorId();
	  ds["state"]=it->second->state();
	  ds["id"]=it->second->status()->id;
	  ds["status"]=it->second->status()->status;
	  ds["slc"]=it->second->status()->slc;
	  ds["gtc"]=it->second->status()->gtc;
	  ds["bcid"]=(Json::Value::UInt64) it->second->status()->bcid;
	  ds["bytes"]=(Json::Value::UInt64)it->second->status()->bytes;
	  ds["host"]=it->second->status()->host;
	  array_slc.append(ds);



	}
      m->setAnswer(array_slc);
      return;
    }
}


void LDIFServer::configure(levbdim::fsmmessage* m)
{

  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();
  uint32_t ctrlreg=m->content()["ctrlreg"].asUInt();
  LOG4CXX_INFO(_logLdaq," Configuring with "<<difid<<" ctr "<<ctrlreg<<" cont "<<m->content());
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  Json::Value array_slc;
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}
      
      itd->second->configure(ctrlreg);
      Json::Value ds;
      ds["id"]=itd->first;
      ds["slc"]=itd->second->status()->slc;
      array_slc.append(ds);
      m->setAnswer(array_slc);
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->configure(ctrlreg);
	  Json::Value ds;
	  ds["id"]=it->first;
	  ds["slc"]=it->second->status()->slc;
	  array_slc.append(ds);


	}
      m->setAnswer(array_slc);
      return;
    }
}

void LDIFServer::start(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}
      this->startDIFThread(itd->second);
      itd->second->start();
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  this->startDIFThread(it->second);
	  it->second->start();
	}

      return;
    }
}
void LDIFServer::stop(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}

      itd->second->stop();
     
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->stop();
	}
      
      return;
    }
}
void LDIFServer::destroy(levbdim::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");

	  return;
	}

      itd->second->destroy();

      return;
    }
  else
    {

      bool running=false;
      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  running=running ||it->second->readoutStarted();
	}
      if (running)
	{
	  for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	    {
	      it->second->setReadoutStarted(false);
	    }

	  this->joinThreads();
	}
      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->destroy();
	}


      return;
    }
}






LDIFServer::LDIFServer(std::string name) 
{

  //_fsm=new levbdim::fsm(name);
  _fsm=new fsmweb(name);

  // Register state
  _fsm->addState("CREATED");
  _fsm->addState("SCANNED");
  _fsm->addState("INITIALISED");
  _fsm->addState("DBREGISTERED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  _fsm->addState("STOPPED");
  _fsm->addState("THRESHOLDSET");
  _fsm->addState("GAINSET");
  _fsm->addTransition("SCAN","CREATED","SCANNED",boost::bind(&LDIFServer::scan, this,_1));
  _fsm->addTransition("INITIALISE","SCANNED","INITIALISED",boost::bind(&LDIFServer::initialise, this,_1));
  _fsm->addTransition("REGISTERDB","INITIALISED","DBREGISTERED",boost::bind(&LDIFServer::registerdb, this,_1));
  _fsm->addTransition("REGISTERDB","DBREGISTERED","DBREGISTERED",boost::bind(&LDIFServer::registerdb, this,_1));
  _fsm->addTransition("REGISTERDB","CONFIGURED","DBREGISTERED",boost::bind(&LDIFServer::registerdb, this,_1));
  _fsm->addTransition("CONFIGURE","DBREGISTERED","CONFIGURED",boost::bind(&LDIFServer::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&LDIFServer::configure, this,_1));
  _fsm->addTransition("CONFIGURE","STOPPED","CONFIGURED",boost::bind(&LDIFServer::configure, this,_1));
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&LDIFServer::start, this,_1));
  _fsm->addTransition("START","STOPPED","RUNNING",boost::bind(&LDIFServer::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","STOPPED",boost::bind(&LDIFServer::stop, this,_1));
  _fsm->addTransition("DESTROY","STOPPED","CREATED",boost::bind(&LDIFServer::destroy, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&LDIFServer::destroy, this,_1));

  _fsm->addTransition("STATUS","SCANNED","SCANNED",boost::bind(&LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","INITIALISED","INITIALISED",boost::bind(&LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","DBREGISTERED","DBREGISTERED",boost::bind(&LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","CONFIGURED","CONFIGURED",boost::bind(&LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","RUNNING","RUNNING",boost::bind(&LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","STOPPED","STOPPED",boost::bind(&LDIFServer::status, this,_1));

  _fsm->addCommand("STATUS",boost::bind(&LDIFServer::cmdStatus,this,_1,_2));
  _fsm->addCommand("SETTHRESHOLD",boost::bind(&LDIFServer::setThreshold,this,_1,_2));

    //Start server
  std::stringstream s0;
  s0.str(std::string());
  s0<<"LDIFServer-"<<name;
  DimServer::start(s0.str().c_str()); 

  memset(theDBDimInfo_,0,255*sizeof(DimInfo*));
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }
}







void LDIFServer::prepareDevices()
{
  for ( std::map<uint32_t,FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    if (it->second!=NULL) delete it->second;
  theFtdiDeviceInfoMap_.clear();
  for ( std::map<uint32_t,LDIF*>::iterator it=theDIFMap_.begin();it!=theDIFMap_.end();it++)
    if (it->second!=NULL) delete it->second;
  theDIFMap_.clear();
  system("/opt/dhcal/bin/ListDevices.py");
  std::string line;
  std::ifstream myfile ("/var/log/pi/ftdi_devices");
  std::stringstream diflist;



  if (myfile.is_open())
    {
      while ( myfile.good() )
	{
	  getline (myfile,line);
	  FtdiDeviceInfo* difi=new FtdiDeviceInfo();
	  memset(difi,0,sizeof(FtdiDeviceInfo));
	  sscanf(line.c_str(),"%x %x %s",&difi->vendorid,&difi->productid,difi->name);
	  if (strncmp(difi->name,"FT101",5)==0)
	    {
	      sscanf(difi->name,"FT101%d",&difi->id); 
	      difi->type=0;
	      std::pair<uint32_t,FtdiDeviceInfo*> p(difi->id,difi);
	      theFtdiDeviceInfoMap_.insert(p);
	    }
	  if (strncmp(difi->name,"DCCCCC",6)==0)
	    {sscanf(difi->name,"DCCCCC%d",&difi->id);difi->type=0x10;}


	}
      myfile.close();
    }
  else 
    {
      std::cout << "Unable to open file"<<std::endl; 
      LOG4CXX_FATAL(_logLdaq," Unable to open /var/log/pi/ftdi_devices");
    }

  for (std::map<uint32_t,FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    printf("Device found and register: %d with info %d %d %s type %d \n", it->first,it->second->vendorid,it->second->productid,it->second->name,it->second->type);
}



void LDIFServer::startDIFThread(LDIF* d)
{
  if (d->readoutStarted()) return;
  d->setReadoutStarted(true);	

  g_d.create_thread(boost::bind(&LDIF::readout,d));
  
}


void LDIFServer::registerDB(std::string state)
{

  for (std::map<uint32_t,LDIF*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
    {
      if (theDBDimInfo_[itd->first]!=NULL) delete theDBDimInfo_[itd->first];
      std::stringstream s;
      s<<"/DB/"<<state<<"/DIF"<<itd->first;
      theDBDimInfo_[itd->first] = new DimInfo(s.str().c_str(),itd->second->dbdif(),sizeof(DIFDbInfo),this);

      itd->second->publishState("DB_REGISTERED");


    }
}

void  LDIFServer::infoHandler( ) 
{
  
  DimInfo *curr = (DimInfo*) getInfo(); // get current DimStampedInfo address
  std::cout<<curr->getName()<<std::endl;
  for (int i=0;i<255;i++)
    {
      if (curr!=theDBDimInfo_[i]) continue;
      if (theDIFMap_.find(i)== theDIFMap_.end()) continue;
      //memcpy(&theDIFDbInfo_[i],curr->getData(),sizeof(DIFDbInfo));
      memcpy(theDIFMap_[i]->dbdif(),curr->getData(),curr->getSize());
      //printf("Dim info read %d %d \n",theDIFMap_[i]->dbdif()->id,theDIFMap_[i]->dbdif()->nbasic);
      LOG4CXX_INFO(_logLdaq,"DIF "<<theDIFMap_[i]->dbdif()->id<<" is read from DB with nasic="<<theDIFMap_[i]->dbdif()->nbasic);
    }
}
/*
void LDIFServer::setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,SingleHardrocV2ConfigurationFrame& ConfigHR2)
{
  printf(" Seuil %d %d %d %x %x %x %x %x\n",B0,B1,B2,ConfigHR2[3],ConfigHR2[4],ConfigHR2[5],ConfigHR2[6],ConfigHR2[7]);
  ConfigHR2[3]= ((B2>>2)&0xFF);
  ConfigHR2[4]= 0;
  ConfigHR2[4]|=((B2&0x03)<<6);
  ConfigHR2[4]|=((B1>>4)&0x3F);
  ConfigHR2[5]= 0;
  ConfigHR2[5]|=((B1&0x0F)<<4);
  ConfigHR2[5]|=((B0>>6)&0x0F);
  ConfigHR2[6]&=0x3;
  ConfigHR2[6]|=((B0&0x3F)<<2);
  printf(" Apres %d %d %d %x %x %x %x %x \n",B0,B1,B2,ConfigHR2[3],ConfigHR2[4],ConfigHR2[5],ConfigHR2[6],ConfigHR2[7]);
}
void LDIFServer::setGain(uint32_t gain,SingleHardrocV2ConfigurationFrame& ConfigHR2)
{
 
  for (uint32_t ip=0;ip<64;ip++)
    ConfigHR2[100-ip]=(gain&0xFF); // Pas |=
}

void LDIFServer::setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,DIFDbInfo& s)
{
  printf(" DIF %d \n",s.id);
  for (int i=0;i<s.nbasic;i++)
    {
      printf("ASIC %d \n",i);
      setThreshold(B0,B1,B2,s.slow[i]);
    }
}
void LDIFServer::setGain(uint32_t gain,DIFDbInfo& s)
{
  for (int i=0;i<s.nbasic;i++)
    setGain(gain,s.slow[i]);
}
void LDIFServer::setThreshold(uint32_t B0,uint32_t B1,uint32_t B2)
{

  for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
    {
      uint32_t difid=itd->first;
      if (theDIFDbInfo_[difid].id==difid)
	{
	  setThreshold(B0,B1,B2,theDIFDbInfo_[difid]);
	}
    }



}
void LDIFServer::setGain(uint32_t gain)
{
  for (std::map<uint32_t,DIFReadout*>::iterator itd=theDIFMap_.begin();itd!=theDIFMap_.end();itd++)
    {
      uint32_t difid=itd->first;
      if (theDIFDbInfo_[difid].id==difid)
       
	{
	  setGain(gain,theDIFDbInfo_[difid]);
	}
    }


}
*/
