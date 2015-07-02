
#include "DimCAENHVServer.h"

DimCAENHVServer::DimCAENHVServer() :thePeriod_(5),storeDb_(false),my_(NULL)
{

  

  std::stringstream s0;
  char hname[80];
  gethostname(hname,80);
  s0<<"/CAENHV/"<<hname<<"/STATUS";

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/CHANNEL";
  channelReadService_ = new DimService(s0.str().c_str(),"I:3,F:3,I:1,C:128,I:1",&currentChannel_,sizeof(HVMONDescription));
  channelReadService_->updateService();

  
  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/INITIALISE";
  initialiseCommand_=new DimCommand(s0.str().c_str(),"C",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/READCHANEL";
  readChannelCommand_=new DimCommand(s0.str().c_str(),"I:1",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/SETV0";
  setV0Command_=new DimCommand(s0.str().c_str(),"I:1,F:1",this);

  s0.str(std::string());
  s0<<"/CAENHV/"<<hname<<"/SETI0";
  setI0Command_=new DimCommand(s0.str().c_str(),"I:1,F:1",this);

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
  Loop();
}


void DimCAENHVServer::Initialise(std::string account,std::string setup)
{
  my_=new MyInterface(account);
  my_>connect();
  theHVRACKMyProxy_= new HVRACKMyProxy(my);
  theSETUPMyProxy_= new SETUPMyProxy(my);
  theDETECTORMyProxy_= new DETECTORMyProxy(my);
  theHVMONMyProxy_= new HVMONMyProxy(my);
  thePTMONMyProxy_= new PTMONMyProxy(my);
  
  std::stringstream s0;
  s0.str(std::string());
  s0<<" NAME="<<setup;
  theSETUPMyProxy_->select(s0.str());
  theSetupId_=theSETUPMyProxy_->getDescription(0).getIDX();

  std::stringstream s0;
  s0.str(std::string());
  s0<<" SETUPID="<<theSetupId_;
  theDETECTORMyProxy_->select(s0.str());
  theHvrackId_=theDETECTORMyProxy_->getDescription(0).getIDX();

  std::stringstream s0;
  s0.str(std::string());
  s0<<" IDX="<<theHvrackId_;
  theHVRACKMyProxy_->select(s0.str());
  
  theHV_= new HVCaenInterface(std::string(theHVRACKMyProxy_->->getDescription(0).getHOSTNAME()),std::string(theHVRACKMyProxy_->->getDescription(0).getUSERNAME()),std::string(theHVRACKMyProxy_->->getDescription(0).getPWD()));
  
}

void DimCAENHVServer::ReadChannel(uint32_t chan)
{
  if (theHV_==NULL) return;
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
  currentChannel.setHVRACKID(theHvrackId_);
  currentChannel.setHVCHANNEL(chan);
  currentChannel.setVSET(theHV_->GetVoltageSet(chan));
  currentChannel.setVMON(theHV_->GetVoltageRead(chan));
  currentChannel.setIMON(theHV_->GetCurrentRead(chan));
  currentChannel.setSTATUS(theHV_->GetStatus(chan));
 
  channelReadService_->updateService(&currentChannel_);
}
void DimCAENHVServer::setV0(uint32_t chan,float v)
{
  if (theHV_==NULL) return;
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
  if (chan!=0xFFFF)
    {
      currentChannel.setHVRACKID(theHvrackId_);
      currentChannel.setHVCHANNEL(chan);
      theHV_->SetVoltage(chan,v);
      currentChannel.setVSET(theHV_->GetVoltageSet(chan));
      currentChannel.setVMON(theHV_->GetVoltageRead(chan));
      currentChannel.setIMON(theHV_->GetCurrentRead(chan));
      currentChannel.setSTATUS(theHV_->GetStatus(chan));
      channelReadService_->updateService(&currentChannel_);
    }
  else
    {
      // Alll channels

      if (theDETECTORMyProxy_==NULL) return;
      std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
      for (std::map<uint32_t,DETECTORDescription>::iterator it=det.begin();it!=det.end();it++)
	{
	  theHV_->SetVoltage(it->getHVCHANNEL(),v);
	}

    }
}
void DimCAENHVServer::setI0(uint32_t chan,float v)
{
  if (theHV_==NULL) return;
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
  if (chan!=0xFFFF)
    {
      currentChannel.setHVRACKID(theHvrackId_);
      currentChannel.setHVCHANNEL(chan);
      theHV_->SetCurrent(chan,v);
      currentChannel.setVSET(theHV_->GetVoltageSet(chan));
      currentChannel.setVMON(theHV_->GetVoltageRead(chan));
      currentChannel.setIMON(theHV_->GetCurrentRead(chan));
      currentChannel.setSTATUS(theHV_->GetStatus(chan));
      channelReadService_->updateService(&currentChannel_);
    }
  else
    {
      // Alll channels

      if (theDETECTORMyProxy_==NULL) return;
      std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
      for (std::map<uint32_t,DETECTORDescription>::iterator it=det.begin();it!=det.end();it++)
	{
	  theHV_->SetVoltageSet(it->getHVCHANNEL(),v);
	}

    }
}
void DimCAENHVServer::setOn(uint32_t chan)
{
  if (theHV_==NULL) return;
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
  if (chan!=0xFFFF)
    {
      currentChannel.setHVRACKID(theHvrackId_);
      currentChannel.setHVCHANNEL(chan);
      theHV_->SetOn(chan);
      currentChannel.setVSET(theHV_->GetVoltageSet(chan));
      currentChannel.setVMON(theHV_->GetVoltageRead(chan));
      currentChannel.setIMON(theHV_->GetCurrentRead(chan));
      currentChannel.setSTATUS(theHV_->GetStatus(chan));
      channelReadService_->updateService(&currentChannel_);
    }
  else
    {
      // Alll channels

      if (theDETECTORMyProxy_==NULL) return;
      std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
      for (std::map<uint32_t,DETECTORDescription>::iterator it=det.begin();it!=det.end();it++)
	{
	  theHV_->SetOn(it->getHVCHANNEL());
	}

    }
}
void DimCAENHVServer::setOff(uint32_t chan)
{
  if (theHV_==NULL) return;
  //  if (theDETECTORMyProxy_==NULL) return;
  // std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
  if (chan!=0xFFFF)
    {
      currentChannel.setHVRACKID(theHvrackId_);
      currentChannel.setHVCHANNEL(chan);
      theHV_->SetOff(chan);
      currentChannel.setVSET(theHV_->GetVoltageSet(chan));
      currentChannel.setVMON(theHV_->GetVoltageRead(chan));
      currentChannel.setIMON(theHV_->GetCurrentRead(chan));
      currentChannel.setSTATUS(theHV_->GetStatus(chan));
      channelReadService_->updateService(&currentChannel_);
    }
  else
    {
      // Alll channels

      if (theDETECTORMyProxy_==NULL) return;
      std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
      for (std::map<uint32_t,DETECTORDescription>::iterator it=det.begin();it!=det.end();it++)
	{
	  theHV_->SetOff(it->getHVCHANNEL());
	}

    }
}

DimCAENHVServer::~DimCAENHVServer()
{
  delete theCAENHV_;
}

void DimCAENHVServer::monitorStart(uint32_t period)
{
  monitorRunning_=true;
  g_d.create_thread(boost::bind(&DimCAENHVServer::readout, this,period));
}

void DimCAENHVServer::storeCurrentChannel()
{

  if (theHVMONMyProxy_==NULL) return;

  memcpy(theHVMONMyProxy_->getCurrent(),&currentChannel_,sizeof(HVMONDescription));
  theHVMONMyProxy_->insert();
}

void DimCAENHVServer::readout(uint32_t period)
{
  while (monitorRunning_ && theDETECTORMyProxy_!=NULL )
    {

      std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
      for (std::map<uint32_t,DETECTORDescription>::iterator it=det.begin();it!=det.end();it++)
	{
	  this->readChannel(it->getHVCHANNEL());
	  this->storeCurrentChannel();
	  
	}



      sleep((unsigned int) period);
    }
}
void DimCAENHVServer::monitorStop()
{
  monitorRunning_=false;
  g_d.join_all();


}


void DimCAENHVServer::regulationStart(uint32_t period)
{
  regulationRunning_=true;
  g_r.create_thread(boost::bind(&DimCAENHVServer::regulate, this,period));
}

void DimCAENHVServer::storeCurrentChannel()
{

  if (theHVMONMyProxy_==NULL) return;

  memcpy(theHVMONMyProxy_->getCurrent(),&currentChannel_,sizeof(HVMONDescription));
  theHVMONMyProxy_->insert();
}
void DimCAENHVServer::regulate(uint32_t period)
{
  while (regulationRunning_ && theDETECTORMyProxy_!=NULL )
    {

      std::map<uint32_t,DETECTORDescription> det=theDETECTORMyProxy_->getMap();
      // Read last P and T Value
      thePTMONMyProxy_->select("NOW()-HEURE<1000");
      std::map<uint32_t,PTMONDescription> ptm=thePTMONMyProxy_->getMap();
      uint32_t nsample=0;float P=0,T=0;
      for (std::map<uint32_t,PTMONDescription>::iterator it=ptm.begin();it!=ptm.end();it++)
	{
	  nsample++;
	  P+=it->getPRESSURE();
	  T+=it->getTEMPERATURE();
	}

      if (nsample<1) {sleep((unsigned int) period); continue;}
      P/=nsample;
      T/=nsample;
     

      for (std::map<uint32_t,DETECTORDescription>::iterator it=det.begin();it!=det.end();it++)
	{
	  // Read values of HV

	  float P0=it->getPREF();
	  float T0=it->getTREF();
	  float HV0=it->getVREF();

	  float Vexpected=HV0*P*T0/(P0*T);
	  // Current value
	  this->readChannel(it->getHVCHANNEL());
	  float vmon=currentChannel_.getVMON();

	  float Veff = vmon*P0/T0*T/P;
                
	  float correction = ((Veff/HV0-1)*100);
	  float vcor =abs(Veff-HV0);

	  if (vcor>20 && vcor<=200)
	    {
	      theHV_->setVoltage(it->getHVCHANNEL(),Vexpected);
	      std::cout<<"REGULATION >>>> Voltage changed on channel"<< it->getHVCHANNEL()<<std::endl;
	      std::cout<<"\t Current Voltage is "<<vmon<<" leading to an effective voltage of "<<Veff<<" where  one expects "<<Vexpected<< "beeing applied"<<std:;endl;

	    }
	  if (vcor>200)
	    {
	      std::cout<<"ERROR >>>> Voltage not changed on channel"<< it->getHVCHANNEL()<<std::endl;
	      std::cout<<"\t Current Voltage is "<<vmon<<" leading to an effective voltage of "<<Veff<<" where  one expects "<<Vexpected<< "beeing applied"<<std:;endl;
	    }
	  

	  
	}



      sleep((unsigned int) period);
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
  if (currCmd==periodCommand_)
    {
      thePeriod_=currCmd->getInt();
    }
  if (currCmd==storeCommand_)
    {
      theAccount_.assign(currCmd->getString());
      my_=new MyInterface(theAccount_);
      storeDb_=true;
    }

}
void DimCAENHVServer::getTemperature()
{
  TemperatureReadValues_=theCAENHV_->CAENHVTemperatureRead();
  TemperatureReadService_->updateService();

}
void DimCAENHVServer::getPression()
{
  PressionReadValues_=theCAENHV_->CAENHVPressionRead();
  PressionReadService_->updateService();

}
