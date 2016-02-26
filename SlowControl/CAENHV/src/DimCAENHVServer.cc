
#include "DimCAENHVServer.h"
#define ZEROCELSIUS 273.15
DimCAENHVServer::DimCAENHVServer() :my_(NULL)
{

  
  //  theMutex_.unlock();
  sem_init(&theMutex_,0,1);
  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/CAENHV/"<<hname<<"/STATUS";

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/CHANNEL";
  channelReadService_ = new DimService(s0.str().c_str(),"I:3;F:3;I:3;C:128;I:1",&currentChannel_,sizeof(HVMONDescription));
  channelReadService_->updateService(&currentChannel_,sizeof(HVMONDescription));

  
  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/INITIALISE";
  initialiseCommand_=new DimCommand(s0.str().c_str(),"C",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/READCHANNEL";
  readChannelCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/SETV0";
  setV0Command_=new DimCommand(s0.str().c_str(),"I:1;F:1",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/SETI0";
  setI0Command_=new DimCommand(s0.str().c_str(),"I:1;F:1",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/SETON";
  setOnCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/SETOFF";
  setOffCommand_=new DimCommand(s0.str().c_str(),"I:1",this);



  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/STARTMONITOR";
  startMonitorCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/STOPMONITOR";
  stopMonitorCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/STARTREGUL";
  startRegulationCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/STOPREGUL";
  stopRegulationCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  gethostname(hname,80);
  s0<<"DimCAENHVServer-"<<hname;
  DimServer::start(s0.str().c_str()); 

}


void DimCAENHVServer::Initialise(std::string account,std::string setup)
{
  my_=new MyInterface(account);
  //my_->connect();
  theHVRACKMyProxy_= new HVRACKMyProxy(my_);
  theSETUPMyProxy_= new SETUPMyProxy(my_);
  theDETECTORMyProxy_= new DETECTORMyProxy(my_);
  theHVMONMyProxy_= new HVMONMyProxy(my_);
  thePTMONMyProxy_= new PTMONMyProxy(my_);
  theDS1820MONMyProxy_= new DS1820MONMyProxy(my_);
  
  std::stringstream s0;
  s0.str(std::string());
  s0<<" NAME=\""<<setup<<"\"";

  std::cout<<"STMT :"<<s0.str()<<std::endl;
  std::cout<<"Setup :"<<setup<<std::endl;
  theSETUPMyProxy_->select(s0.str());
  std::map<uint32_t,SETUPDescription*> stm=theSETUPMyProxy_->getMap();
  
  theSetupId_=stm.begin()->second->getIDX();

  s0.str(std::string());
  s0<<" SETUPID="<<theSetupId_;
  theDETECTORMyProxy_->select(s0.str());
  std::map<uint32_t,DETECTORDescription*> detm=theDETECTORMyProxy_->getMap();
  theHvrackId_=detm.begin()->second->getHVRACKID();

  s0.str(std::string());
  s0<<" IDX="<<theHvrackId_;
  theHVRACKMyProxy_->select(s0.str());
  std::map<uint32_t,HVRACKDescription*> hvm=theHVRACKMyProxy_->getMap();
  theHV_= new HVCaenInterface(std::string(hvm.begin()->second->getHOSTNAME()),std::string(hvm.begin()->second->getUSERNAME()),std::string(hvm.begin()->second->getPWD()));


  printf("%s %s %s \n",hvm.begin()->second->getHOSTNAME(),hvm.begin()->second->getUSERNAME(),hvm.begin()->second->getPWD());

  //  my_->disconnect();
}

void DimCAENHVServer::ReadChannel(uint32_t chan)
{
  if (theHV_==NULL) return;
  if (!theHV_->isConnected()) theHV_->Connect();
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
  // std::cout<<theHV_->GetVoltageSet(chan)<<std::endl;
  // std::cout<<theHV_->GetVoltageRead(chan)<<std::endl;
  // std::cout<<theHV_->GetCurrentRead(chan)<<std::endl;
  // std::cout<<theHV_->GetStatus(chan)<<std::endl;

  if (theHV_->GetStatus(chan)<0)
    {
      theHV_->Disconnect();
      sleep((unsigned int) 4);
      theHV_->Connect();
    }
  currentChannel_.setHVRACKID(theHvrackId_);
  currentChannel_.setHVCHANNEL(chan);
  currentChannel_.setVSET(theHV_->GetVoltageSet(chan));
  currentChannel_.setVMON(theHV_->GetVoltageRead(chan));
  currentChannel_.setIMON(theHV_->GetCurrentRead(chan));
  currentChannel_.setSTATUS(theHV_->GetStatus(chan));
  currentChannel_.setVALID(1);
  //theHV_->Disconnect();
  printf("%d %f %f %f %f %d %d \n",currentChannel_.getHVCHANNEL(),currentChannel_.getVSET(),currentChannel_.getVMON(),currentChannel_.getIMON(),currentChannel_.getSTATUS(),sizeof(HVMONDescription));
  channelReadService_->updateService(&currentChannel_,sizeof(HVMONDescription));
}
void DimCAENHVServer::setV0(uint32_t chan,float v)
{
  if (theHV_==NULL) return;
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
  if (!theHV_->isConnected()) theHV_->Connect();
  if (chan!=0xFFFF)
    {
      currentChannel_.setHVRACKID(theHvrackId_);
      currentChannel_.setHVCHANNEL(chan);
      theHV_->SetVoltage(chan,v);
      currentChannel_.setVSET(theHV_->GetVoltageSet(chan));
      currentChannel_.setVMON(theHV_->GetVoltageRead(chan));
      currentChannel_.setIMON(theHV_->GetCurrentRead(chan));
      currentChannel_.setSTATUS(theHV_->GetStatus(chan));
      channelReadService_->updateService(&currentChannel_,sizeof(HVMONDescription));
    }
  else
    {
      // Alll channels

      if (theDETECTORMyProxy_==NULL) return;
      std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
      for (std::map<uint32_t,DETECTORDescription*>::iterator it=det.begin();it!=det.end();it++)
	{
	  theHV_->SetVoltage(it->second->getHVCHANNEL(),v);
	}

    }
  //theHV_->Disconnect();
}
void DimCAENHVServer::setI0(uint32_t chan,float v)
{
  if (theHV_==NULL) return;
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
  if (!theHV_->isConnected()) theHV_->Connect();
  if (chan!=0xFFFF)
    {
      currentChannel_.setHVRACKID(theHvrackId_);
      currentChannel_.setHVCHANNEL(chan);
      theHV_->SetCurrent(chan,v);
      currentChannel_.setVSET(theHV_->GetVoltageSet(chan));
      currentChannel_.setVMON(theHV_->GetVoltageRead(chan));
      currentChannel_.setIMON(theHV_->GetCurrentRead(chan));
      currentChannel_.setSTATUS(theHV_->GetStatus(chan));
      channelReadService_->updateService(&currentChannel_,sizeof(HVMONDescription));
    }
  else
    {
      // Alll channels

      if (theDETECTORMyProxy_==NULL) return;
      std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
      for (std::map<uint32_t,DETECTORDescription*>::iterator it=det.begin();it!=det.end();it++)
	{
	  theHV_->SetVoltage(it->second->getHVCHANNEL(),v);
	}

    }
  //theHV_->Disconnect();
}
void DimCAENHVServer::setOn(uint32_t chan)
{
  if (theHV_==NULL) return;
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
  if (!theHV_->isConnected()) theHV_->Connect();
  if (chan!=0xFFFF)
    {
      currentChannel_.setHVRACKID(theHvrackId_);
      currentChannel_.setHVCHANNEL(chan);
      theHV_->SetOn(chan);
      currentChannel_.setVSET(theHV_->GetVoltageSet(chan));
      currentChannel_.setVMON(theHV_->GetVoltageRead(chan));
      currentChannel_.setIMON(theHV_->GetCurrentRead(chan));
      currentChannel_.setSTATUS(theHV_->GetStatus(chan));
      channelReadService_->updateService(&currentChannel_,sizeof(HVMONDescription));
    }
  else
    {
      // Alll channels

      if (theDETECTORMyProxy_==NULL) return;
      std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
      for (std::map<uint32_t,DETECTORDescription*>::iterator it=det.begin();it!=det.end();it++)
	{
	  theHV_->SetOn(it->second->getHVCHANNEL());
	}

    }
  //theHV_->Disconnect();
}
void DimCAENHVServer::setOff(uint32_t chan)
{
  if (theHV_==NULL) return;
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
  if (!theHV_->isConnected()) theHV_->Connect();
  if (chan!=0xFFFF)
    {
      std::cout<<" Channel "<<chan <<" is turned off "<<std::endl;
      currentChannel_.setHVRACKID(theHvrackId_);
      currentChannel_.setHVCHANNEL(chan);
      theHV_->SetOff(chan);
      currentChannel_.setVSET(theHV_->GetVoltageSet(chan));
      currentChannel_.setVMON(theHV_->GetVoltageRead(chan));
      currentChannel_.setIMON(theHV_->GetCurrentRead(chan));
      currentChannel_.setSTATUS(theHV_->GetStatus(chan));
      channelReadService_->updateService(&currentChannel_,sizeof(HVMONDescription));
    }
  else
    {
      // Alll channels

      if (theDETECTORMyProxy_==NULL) return;
      std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
      for (std::map<uint32_t,DETECTORDescription*>::iterator it=det.begin();it!=det.end();it++)
	{
	  theHV_->SetOff(it->second->getHVCHANNEL());
	}

    }
  //theHV_->Disconnect();
}

DimCAENHVServer::~DimCAENHVServer()
{
  delete theHV_;
}

void DimCAENHVServer::monitorStart(uint32_t period)
{
  monitorRunning_=true;
  g_d.create_thread(boost::bind(&DimCAENHVServer::readout, this,period));
}

void DimCAENHVServer::storeCurrentChannel()
{
  // return;
  if (theHVMONMyProxy_==NULL) return;

  memcpy(theHVMONMyProxy_->getCurrent(),&currentChannel_,sizeof(HVMONDescription));
  theHVMONMyProxy_->insert();
  return;

  std::stringstream ss;
  ss<<"INSERT INTO HVMON (HVRACKID,HVCHANNEL,VSET,VMON,IMON,STATUS) VALUES ("<<
    currentChannel_.getHVRACKID()<<","<<
    currentChannel_.getHVCHANNEL()<<","<<
    currentChannel_.getVSET()<<","<<
    currentChannel_.getVMON()<<","<<
    currentChannel_.getIMON()<<","<<
    currentChannel_.getSTATUS()<<")";
    
  std::cout<<ss.str()<<std::endl;
  my_->connect();
  my_->executeQuery(ss.str());
  my_->disconnect();
  std::cout<<ss.str()<<" is done "<<std::endl;
  
}

void DimCAENHVServer::readout(uint32_t period)
{
  while (monitorRunning_ && theDETECTORMyProxy_!=NULL )
    {
      if (true)
	{
	  printf("Lock %s \n",__FUNCTION__);
	  sem_wait(&theMutex_);
	  std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
	  for (std::map<uint32_t,DETECTORDescription*>::iterator it=det.begin();it!=det.end();it++)
	    {
	      this->ReadChannel(it->second->getHVCHANNEL());
	      this->storeCurrentChannel();
	  
	    }
	  sem_post(&theMutex_); printf("Unlock %s \n",__FUNCTION__);
	  



	  sleep((unsigned int) period);
      	}
      else
	{
	  printf("Cannot lock %s \n",__FUNCTION__);
	  sleep(5);
	}
    }
  printf("On a fini \n");
}
void DimCAENHVServer::monitorStop()
{
  monitorRunning_=false;
  printf("On stoppe \n");
  //g_d.join_all();


}


void DimCAENHVServer::regulationStart(uint32_t period)
{
  regulationRunning_=true;
  g_r.create_thread(boost::bind(&DimCAENHVServer::regulate, this,period));
}

void DimCAENHVServer::regulate(uint32_t period)
{
  while (regulationRunning_ && theDETECTORMyProxy_!=NULL )
    {
      if (true)
	{
	  printf("Lock %s \n",__FUNCTION__);

	  sem_wait(&theMutex_);
	  std::map<uint32_t,DETECTORDescription*> det=theDETECTORMyProxy_->getMap();
	  // Read last P and T Value
	  thePTMONMyProxy_->select("NOW()-HEURE<1000");
	  std::map<uint32_t,PTMONDescription*> ptm=thePTMONMyProxy_->getMap();
	  uint32_t nsample=0;float P=0,T=0;
	  for (std::map<uint32_t,PTMONDescription*>::iterator it=ptm.begin();it!=ptm.end();it++)
	    {
	      nsample++;
	      P+=it->second->getPRESSURE();
	      T+=it->second->getTEMPERATURE();
	    }

	  if (nsample<1) { sem_post(&theMutex_);printf("Unlock %s \n",__FUNCTION__);sleep((unsigned int) period); continue;}
	  P/=nsample;
	  T/=nsample;
	  T+=ZEROCELSIUS;

	  // Read last P and T Value
	  theDS1820MONMyProxy_->select("NOW()-HEURE<1000");
	  std::map<uint32_t,DS1820MONDescription*> ttm=theDS1820MONMyProxy_->getMap();
	  uint32_t nsample1=0;float TIN=0,TOUT=0;
	  for (std::map<uint32_t,DS1820MONDescription*>::iterator it=ttm.begin();it!=ttm.end();it++)
	    {
	      nsample1++;
	      TIN+=it->second->getTIN();
	      TOUT+=it->second->getTOUT();
	    }

	  if (nsample1<1) { sem_post(&theMutex_);printf("Unlock %s \n",__FUNCTION__);sleep((unsigned int) period); continue;}
	  TIN/=nsample1;
	  TOUT/=nsample1;
	  TOUT+=ZEROCELSIUS;

	  // Force T to TOUT
	  T=TOUT;
      
	  for (std::map<uint32_t,DETECTORDescription*>::iterator it=det.begin();it!=det.end();it++)
	    {
	      // Read values of HV

	      float P0=it->second->getPREF();
	      float T0=it->second->getTREF()+ZEROCELSIUS;
	      float HV0=it->second->getVREF();
	      printf("Expected values are %f at P=%f and T=%f ",HV0,P0,T0);
	      float Vexpected=HV0*P*T0/(P0*T);
	      printf("That gives %f V at P=%f and T=%f \n",Vexpected,P,T);
	      // Current value
	      this->ReadChannel(it->second->getHVCHANNEL());
	      float vmon=currentChannel_.getVMON();

	      float Veff = vmon*P0/T0*T/P;
                
	      float correction = ((Veff/HV0-1)*100);
	      float vcor =abs(Veff-HV0);

	      if (vcor>20 && vcor<=200)
		{
		  if (!theHV_->isConnected()) theHV_->Connect();
		  theHV_->SetVoltage(it->second->getHVCHANNEL(),Vexpected);
		  std::cout<<"REGULATION >>>> Voltage changed on channel"<< it->second->getHVCHANNEL()<<std::endl;
		  std::cout<<"\t Current Voltage is "<<vmon<<" leading to an effective voltage of "<<Veff<<" where  one expects "<<Vexpected<< "beeing applied"<<std::endl;
		  //theHV_->Disconnect();
		}
	      if (vcor>200)
		{
		  std::cout<<"ERROR >>>> Voltage not changed on channel"<< it->second->getHVCHANNEL()<<std::endl;
		  std::cout<<"\t Current Voltage is "<<vmon<<" leading to an effective voltage of "<<Veff<<" where  one expects "<<Vexpected<< "beeing applied"<<std::endl;
		}
	  

	  
	    }

	  sem_post(&theMutex_);printf("Unlock %s \n",__FUNCTION__);

	  sleep((unsigned int) period);
     	}
      else
	{
	  printf("Cannot lock %s \n",__FUNCTION__);
	  sleep(5);
	} 
    }
}

void DimCAENHVServer::regulationStop()
{
  regulationRunning_=false;
  g_r.join_all();


}


void DimCAENHVServer::commandHandler()
{
  DimCommand *currCmd = getCommand();
  printf(" J'ai recu %s COMMAND  \n",currCmd->getName());
  if (currCmd==initialiseCommand_)
    {
      char* sdb=getenv("SLOWDB");
      std::string s;s.assign(sdb);

      //std::string s;s.assign(currCmd->getString());
      int ipass = s.find(";");
      theAccount_.clear();
      theAccount_=s.substr(0,ipass); 
      theSetup_=s.substr(ipass+1,s.size()-ipass);
      this->Initialise(theAccount_,theSetup_);
    }
  if (currCmd==readChannelCommand_)
    {
      this->ReadChannel(currCmd->getInt());
    }
  if (currCmd==setV0Command_)
    {
      int* idat=(int*) currCmd->getData();
      float* fdat=(float*) currCmd->getData();
      this->setV0(idat[0],fdat[1]);
    }
  if (currCmd==setI0Command_)
    {
      int* idat=(int*) currCmd->getData();
      float* fdat=(float*) currCmd->getData();
      this->setI0(idat[0],fdat[1]);
    }
  if (currCmd==setOnCommand_)
    {
      this->setOn(currCmd->getInt());
    }
  if (currCmd==setOffCommand_)
    {
      this->setOff(currCmd->getInt());
    }
  if (currCmd==startMonitorCommand_)
    {
      theMonitorPeriod_=currCmd->getInt();
      this->monitorStart(theMonitorPeriod_);
    }
  if (currCmd==stopMonitorCommand_)
    {
      this->monitorStop();
    }
  if (currCmd==startRegulationCommand_)
    {
      theRegulationPeriod_=currCmd->getInt();
      this->regulationStart(theRegulationPeriod_);
    }
  if (currCmd==stopRegulationCommand_)
    {
      this->regulationStop();
    }

}
