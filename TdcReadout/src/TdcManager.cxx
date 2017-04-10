#include "TdcManager.hh"
using namespace lytdc;
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <boost/format.hpp>
char* CurlQuery(char* AddURL,char* Chaine);
extern  int alphasort(); //Inbuilt sorting function  
std::string wget(std::string url);

int file_select_tdcmanager(const struct direct *entry)  
{  
  if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))  
    return (0);  
  else  
    return (1);  
}  
TdcManager::TdcManager(std::string name) : levbdim::baseApplication(name), _group(NULL),_sCtrl(NULL),_sTDC1(NULL),_sTDC2(NULL)
{
  //_fsm=new lytdc::fsm(name);
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&TdcManager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&TdcManager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&TdcManager::configure, this,_1));
  
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&TdcManager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&TdcManager::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&TdcManager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&TdcManager::destroy, this,_1));
  
  
  
  //_fsm->addCommand("JOBLOG",boost::bind(&TdcManager::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS",boost::bind(&TdcManager::c_status,this,_1,_2));
  _fsm->addCommand("DIFLIST",boost::bind(&TdcManager::c_diflist,this,_1,_2));
  _fsm->addCommand("SET6BDAC",boost::bind(&TdcManager::c_set6bdac,this,_1,_2));
  _fsm->addCommand("SETVTHTIME",boost::bind(&TdcManager::c_setvthtime,this,_1,_2));
  _fsm->addCommand("SETMASK",boost::bind(&TdcManager::c_setMask,this,_1,_2));
  
  
  
 
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
  std::stringstream s0;
  s0.str(std::string());
  s0<<"TdcManager-"<<name;
  DimServer::start(s0.str().c_str()); 

 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink
  NL::init();
  
}
void TdcManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";
   if (_msh==NULL) return;
  Json::Value jl;
  for (uint32_t i=0;i<2;i++)
    {
      if (_msh->tdc(i)==NULL) continue;
      Json::Value jt;
      jt["detid"]=_msh->tdc(i)->detectorId();
      jt["sourceid"]=_msh->tdc(i)->difId();
      jt["gtc"]=_msh->tdc(i)->gtc();
      jt["abcid"]=(Json::Value::UInt64)_msh->tdc(i)->abcid();
      jt["event"]=_msh->tdc(i)->event();
      jl.append(jt);
    }
  response["TDCSTATUS"]=jl;
}
void TdcManager::c_diflist(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";
  response["DIFLIST"]="EMPTY";
  if (_msh==NULL) return;
  Json::Value jl;
  for (uint32_t i=0;i<2;i++)
    {
      if (_msh->tdc(i)==NULL) continue;
      Json::Value jt;
      jt["detid"]=_msh->tdc(i)->detectorId();
      jt["sourceid"]=_msh->tdc(i)->difId();
      jl.append(jt);
    }
  response["DIFLIST"]=jl;
}

void TdcManager::c_set6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  uint32_t nc=atol(request.get("value","31").c_str());
  
  this->set6bDac(nc&0xFF);
  response["6BDAC"]=nc;
}
void TdcManager::c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  uint32_t nc=atol(request.get("value","380").c_str());
  
  this->setVthTime(nc);
  response["VTHTIME"]=nc;
}
void TdcManager::c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  uint32_t nc=atol(request.get("value","4294967295").c_str());
  
  this->setMask(nc);
  response["MASK"]=nc;
}
void TdcManager::initialise(levbdim::fsmmessage* m)
{
  ///LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;

  uint32_t pSLC=0,pTDC1=0,pTDC2=0;
  std::string hSLC=std::string("");
  std::string hTDC1=std::string("");
  std::string hTDC2=std::string("");
   if (m->content().isMember("socket"))
    { 

      pSLC=m->content()["socket"]["portSLC"].asInt();
      hSLC=m->content()["socket"]["hostSLC"].asString();
  
      pTDC1=m->content()["socket"]["portTDC1"].asInt();
      hTDC1=m->content()["socket"]["hostTDC1"].asString();
  
  
      pTDC2=m->content()["socket"]["portTDC2"].asInt();
      hTDC2=m->content()["socket"]["hostTDC2"].asString();

      this->parameters()["socket"]=m->content()["socket"];
    }
   else
     {
       pSLC=this->parameters()["socket"]["portSLC"].asInt();
      hSLC=this->parameters()["socket"]["hostSLC"].asString();
  
      pTDC1=this->parameters()["socket"]["portTDC1"].asInt();
      hTDC1=this->parameters()["socket"]["hostTDC1"].asString();
  
  
      pTDC2=this->parameters()["socket"]["portTDC2"].asInt();
      hTDC2=this->parameters()["socket"]["hostTDC2"].asString();
  
     }
  
  
  if (_group!=NULL) delete _group;
  _group=new NL::SocketGroup();
  _msh =new TdcMessageHandler("/dev/shm");
  _onRead= new lytdc::OnRead(_msh);
  _onClientDisconnect= new lytdc::OnClientDisconnect();
  _onDisconnect= new lytdc::OnDisconnect(_msh);
  
  _onAccept=new lytdc::OnAccept(_msh);
  
  _group->setCmdOnRead(_onRead);
  _group->setCmdOnAccept(_onAccept);
 // _group->setCmdOnDisconnect(_onDisconnect);
  _group->setCmdOnDisconnect(_onClientDisconnect);
  //return;
  std::cout<<hSLC<<":"<<pSLC<<std::endl;
  if (_sCtrl!=NULL) delete _sCtrl;
  try {
  _sCtrl=new NL::Socket(hSLC.c_str(),pSLC);
  _group->add(_sCtrl);
   this->startAcquisition(false);
  }
  catch (NL::Exception e)
  {
     std::stringstream s;
     s<<e.msg()<<" SLC socket "<<hSLC<<":"<<pSLC;
     std::cout<<s.str()<<std::endl;
    Json::Value array;
   array["ERROR"]=s.str();

   m->setAnswer(array);
    return;
  }
  std::cout<<hTDC1<<":"<<pTDC1<<std::endl;
  
  if (_sTDC1!=NULL) delete _sTDC1;
  try {
  _sTDC1=new NL::Socket(hTDC1.c_str(),pTDC1);
  _group->add(_sTDC1);
  _msh->setMezzanine(1,hTDC1);
  }
  catch (NL::Exception e)
  {
     std::stringstream s;
     s<<e.msg()<<" TDC1 socket "<<hTDC1<<":"<<pTDC1;
     std::cout<<s.str()<<std::endl;
    Json::Value array;
   array["ERROR"]=s.str();

   m->setAnswer(array);
    return;
  }
  #define USE_TDC2
  #ifdef USE_TDC2
  std::cout<<hTDC2<<":"<<pTDC2<<std::endl;
  try {
  if (_sTDC2!=NULL) delete _sTDC2;
  _sTDC2=new NL::Socket(hTDC2.c_str(),pTDC2);
  _group->add(_sTDC2);
  _msh->setMezzanine(2,hTDC2);
  }
 catch (NL::Exception e)
  {
     std::stringstream s;
     s<<e.msg()<<" TDC2 socket "<<hTDC2<<":"<<pTDC2;
     std::cout<<s.str()<<std::endl;
    Json::Value array;
   array["ERROR"]=s.str();

   m->setAnswer(array);
    return;
  }
  #endif
  this->listen();
  disconnected_=0;
  
}

void TdcManager::dolisten()
{
 
       while(true) {

                if(!_group->listen(2000))
		  std::cout << "\nNo msg recieved during the last 2 seconds";
        }

    
}
void TdcManager::listen()
{
  g_store.create_thread(boost::bind(&TdcManager::dolisten, this));
  _running=true;
  // Comment out for LEVBDIM running
  //g_run.create_thread(boost::bind(&TdcManager::doStart, this));

}
void TdcManager::configure(levbdim::fsmmessage* m)
{
  //LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  this->startAcquisition(false);

  std::string ffile=std::string("");
  std::string furl=std::string("");
  if (m->content().isMember("file"))
    { 
      ffile=m->content()["file"].asString();
      this->parameters()["file"]=m->content()["file"];
    }
  else
    if (m->content().isMember("url"))
      {
	furl=m->content()["url"].asString();
	this->parameters()["url"]=m->content()["url"];
      }
    else
      if (this->parameters().isMember("file"))
	{
	  ffile=this->parameters()["file"].asString();
	}
      else
	if (this->parameters().isMember("url"))
	  {
	    furl=this->parameters()["url"].asString();
	  }
	else
	  {
	    std::cout<<" No configuration given"<<std::endl;
	    Json::Value r;
	    r["ERROR"]="No configuration given (file or url)";

	    m->setAnswer(r);
	    return;
	  }
  
  //std::string config=m->content()["configFile"].asString();
  
  // Read the file
  Json::Value jall;
  if (ffile.length()>2)
    {
  //PRSlow::loadAsics(config,_s1,_s2);
      Json::Reader reader;
      std::ifstream ifs (ffile.c_str(), std::ifstream::in);
      Json::Value jall;
      bool parsingSuccessful = reader.parse(ifs,jall,false);
    }
  else
    if (furl.length()>2)
      {
	std::string jsconf=wget(furl);
	std::cout<<jsconf<<std::endl;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(jsconf,jall);
      
      }
  _s1.setJson(jall["ASIC1"]);
  _s1.dumpJson();
  _s1.Print();
  _s2.setJson(jall["ASIC2"]);
  _s2.dumpJson();
  _s2.Print();

  //  _s1.setVthDiscriCharge(900);
  //_s2.setVthDiscriCharge(900);
  // _s1.setVthTime(900);
  //_s2.setVthTime(900);
  _s2.prepare4Tdc(_slcAddr,_slcBuffer);
  _s1.prepare4Tdc(_slcAddr,_slcBuffer,80);
  //s2.prepare4Tdc(adr,val,80);
  _slcBytes=160;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;

  
  //this->parseConfig(config);
  // Now write data
  this->writeRamAvm();
  //this->queryCRC();
  // Do it twice
  this->writeRamAvm();
}

void TdcManager::set6bDac(uint8_t dac)
{
  //this->startAcquisition(false);
  ::sleep(1);
  for (int i=0;i<32;i++)
    {
      _s1.set6bDac(i,dac);
      _s2.set6bDac(i,dac);
    }
  _s2.prepare4Tdc(_slcAddr,_slcBuffer);
  _s1.prepare4Tdc(_slcAddr,_slcBuffer,80);
  //s2.prepare4Tdc(adr,val,80);
  _slcBytes=160;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;
  this->writeRamAvm();

  // do it twice
  this->writeRamAvm();

  // store an "event"
  this->storeSlowControl(0x100);
  //this->startAcquisition(true);
  ::sleep(1);

}
void TdcManager::setMask(uint32_t mask)
{
  //this->startAcquisition(false);
  ::sleep(1);
  for (int i=0;i<32;i++)
    {
      if ((mask>>i)&1)
	{
	  _s1.setMaskDiscriTime(i,0);
	  _s2.setMaskDiscriTime(i,0);
	}
      else
	{
	  _s1.setMaskDiscriTime(i,1);
	  _s2.setMaskDiscriTime(i,1);
	}
    }
  _s2.prepare4Tdc(_slcAddr,_slcBuffer);
  _s1.prepare4Tdc(_slcAddr,_slcBuffer,80);
  //s2.prepare4Tdc(adr,val,80);
  _slcBytes=160;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;
  this->writeRamAvm();

  // do it twice
  this->writeRamAvm();

  // store an "event"
  this->storeSlowControl(0x100);
  //this->startAcquisition(true);
  ::sleep(1);

}

void TdcManager::setVthTime(uint32_t vth)
{
  _s1.setVthTime(vth);
  _s2.setVthTime(vth);
  
  _s2.prepare4Tdc(_slcAddr,_slcBuffer);
  _s1.prepare4Tdc(_slcAddr,_slcBuffer,80);
  //s2.prepare4Tdc(adr,val,80);
  _slcBytes=160;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;
  this->writeRamAvm();

  // do it twice
  this->writeRamAvm();

  // store an "event"
  this->storeSlowControl(0x101);
}

void TdcManager::storeSlowControl(uint32_t mezid)
{
  uint8_t buf[0x1000];
  int info[4];
  info[0]=_run;
  info[1]=mezid;
  info[2]=(_currentGTC[0]&0xFFFF)|((_currentGTC[1]&0xFFFF)<<16);;
  uint32_t size_buf=2*20*sizeof(uint32_t);
  info[3]=size_buf;
  int ier=write(_fdOut,info,4*sizeof(uint32_t));
  memcpy(buf,_s1.ptr(),20*sizeof(uint32_t));
  memcpy(&buf[20*sizeof(uint32_t)],_s2.ptr(),20*sizeof(uint32_t));
  ier=write(_fdOut,buf,size_buf);
}
void TdcManager::sendTrigger(uint32_t nt)
{
  char RC[50000];  //À changer selon vos besoin
  memset(RC,0,50000);
  #ifdef LABJACKUSED
  std::stringstream sq;sq<<"http://lyoac29:54000/LABJACK/CMD?name=PULSE&npulse="<<nt;
  std::cout<<sq.str()<<std::endl;
  CurlQuery((char*) sq.str().c_str(),RC);
  std::cout<<std::string(RC)<<std::endl;
  #endif
  std::stringstream sq;sq<<"http://lyoac29:41000/Mdcc-lyoac29/CMD?name=PAUSE";
  std::cout<<sq.str()<<std::endl;
  CurlQuery((char*) sq.str().c_str(),RC);
  std::cout<<std::string(RC)<<std::endl;
  sq.str(std::string());
  sq<<"http://lyoac29:41000/Mdcc-lyoac29/CMD?name=SETCALIBCOUNT&nclock="<<nt;
  std::cout<<sq.str()<<std::endl;
  CurlQuery((char*) sq.str().c_str(),RC);
  std::cout<<std::string(RC)<<std::endl;
  sq.str(std::string());
  sq<<"http://lyoac29:41000/Mdcc-lyoac29/CMD?name=RELOADCALIB";
  std::cout<<sq.str()<<std::endl;
  CurlQuery((char*) sq.str().c_str(),RC);
  std::cout<<std::string(RC)<<std::endl; 
  sq.str(std::string());
  sq<<"http://lyoac29:41000/Mdcc-lyoac29/CMD?name=RESUME";
  std::cout<<sq.str()<<std::endl;
  CurlQuery((char*) sq.str().c_str(),RC);
  std::cout<<std::string(RC)<<std::endl;
 
  
}
void TdcManager::loop6BDac(uint8_t dacmin,uint8_t dacmax,uint8_t dacstep,uint8_t ngtc)
{
  char RC[50000];  //À changer selon vos besoin
  memset(RC,0,50000);
  _currentGTC[0]=_currentGTC[1]=0;
  
  this->startAcquisition(false);
  for (uint16_t dac=dacmin;dac<dacmax;)
    {
      if (!_loop) break;
      // Stop the TDC
      this->startAcquisition(false);
      // Change DAC value
      set6bDac(dac);
      ::sleep(5);
#ifdef OLDLOOP
      // Wait for ngtc
      //uint32_t fgtc=_currentGTC;
      _firstEvent[0]=0;
      _firstEvent[1]=0;
      std::cout<<"======================> Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" DAC "<<dac<<std::endl;
      
      // Reenable for ngtc
      this->startAcquisition(true);
      while (!(_firstEvent[0]&&_firstEvent[1])) 
	{
	  sendTrigger(1);
	  ::sleep(1);
	}
      //
      //sleep((uint32_t) 5);
      std::cout<<"======================> First event passed Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" DAC "<<dac<<std::endl;
      while (((_currentGTC[0]-_firstEvent[0])<ngtc || (_currentGTC[1]-_firstEvent[1])<ngtc )  && _loop)
	{usleep(100000);
	  std::cout<<"Waiting ======================>  event passed Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" for DAC "<<dac<<" and ngtc"<<ngtc<<std::endl;
	  std::stringstream sq;sq<<"http://lyoac29:54000/LABJACK/CMD?name=PULSE&npulse="<<(int) 1;
	  std::cout<<sq.str()<<std::endl;
	  CurlQuery((char*) sq.str().c_str(),RC);
	  std::cout<<std::string(RC)<<std::endl;

	}
#else
      usleep(200000);//getchar();
      this->startAcquisition(false);
     
      // Wait for ngtc
      //uint32_t fgtc=_currentGTC;
      _firstEvent[0]=0;
      _firstEvent[1]=0;
      std::cout<<"======================> Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" vthTime "<<dac<<std::endl;
      getchar();
      // Reenable for ngtc
      this->startAcquisition(true);
      if (false)
	while (!(_firstEvent[0]&&_firstEvent[1])) 
	{

	  printf ("%d %d | %d %d Reenabling for 5 \n",_firstEvent[0],_currentGTC[0],_firstEvent[1],_currentGTC[1]);
	  sendTrigger(ngtc+1);
	  ::sleep(1);
	}
      else
	{
	  _firstEvent[0]=_firstEvent[1]=0;
	  _currentGTC[0]=_currentGTC[1]=0;
	}
      //
      //sleep((uint32_t) 5);
      std::cout<<"======================> First event passed Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" DAC "<<dac<<std::endl;
      int nallow=ngtc+2;
      uint32_t ts=5;

      int lastgtc[2];
     

     
      
      while (((_currentGTC[0]-_firstEvent[0])<ngtc || (_currentGTC[1]-_firstEvent[1])<ngtc )  && _loop)
	{usleep(100000);

	  /*
	   lastgtc[0]=_currentGTC[0];
	   lastgtc[1]=_currentGTC[1];
	   std::stringstream sq;sq<<"http://lyoac29:54000/LABJACK/CMD?name=PULSE&npulse="<<(int) 1;
	   std::cout<<sq.str()<<std::endl;
	   CurlQuery((char*) sq.str().c_str(),RC);
	   std::cout<<std::string(RC)<<std::endl;
	   while (_currentGTC[0]!=lastgtc[0]+1) {usleep(20000);std::cout<<_currentGTC[0]<<" 0  "<<lastgtc[0]<<std::endl;}
	   while (_currentGTC[1]!=lastgtc[1]+1) {usleep(20000);std::cout<<_currentGTC[1]<<" 1 "<<lastgtc[1]<<std::endl;}
	  */




	  
	  std::cout<<"Waiting ======================>  event passed Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" for VthTime "<<dac<<" and ngtc"<<ngtc<<std::endl;
	  sendTrigger(nallow);
	  //::sleep(ts);
	  getchar();
	  int nl=0;
	  while ((_currentGTC[0]-_firstEvent[0])<ngtc ) {usleep(20000);std::cout<<_currentGTC[0]-_firstEvent[0]<<std::endl;nl++; if (nl>10) {nallow=ngtc-(_currentGTC[0]-_firstEvent[0]);break;}}
	  nl=0;
	  while ((_currentGTC[1]-_firstEvent[1])<ngtc ) {usleep(20000);std::cout<<_currentGTC[1]-_firstEvent[1]<<std::endl;nl++;if (nl>10)  {nallow=ngtc-(_currentGTC[1]-_firstEvent[1]);break;}}
	  //nallow=2;
	  ts=1;
	  
	}

      
#endif
      std::cout<<"======================> End loop Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" DAC "<<dac<<std::endl;
      this->startAcquisition(false);
      //getchar();
      dac+=dacstep;
    }
}

void TdcManager::loopVthTime(uint32_t thmin,uint32_t thmax,uint32_t thstep,uint8_t ngtc)
{
  char RC[50000];  //À changer selon vos besoin
  memset(RC,0,50000);
  _currentGTC[0]=_currentGTC[1]=0;
  
  this->startAcquisition(false);
  for (uint16_t dac=thmax;dac>=thmin;)
    {
      if (!_loop) break;
      // Stop the TDC
      this->startAcquisition(false);
      // Change DAC value
      setVthTime(dac);
      usleep(200000);//getchar();
      // Wait for ngtc
      //uint32_t fgtc=_currentGTC;
      _firstEvent[0]=0;
      _firstEvent[1]=0;
      std::cout<<"======================> Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" vthTime "<<dac<<std::endl;
      
      // Reenable for ngtc
      this->startAcquisition(true);
      for (int i=0;i<10;i++) {this->startAcquisition(true);usleep(10000);}

      if (false)
	while (!(_firstEvent[0]&&_firstEvent[1])) 
	{

	  printf ("%d %d | %d %d Reenabling for 5 \n",_firstEvent[0],_currentGTC[0],_firstEvent[1],_currentGTC[1]);
	  std::stringstream sq;sq<<"http://lyoac29:54000/LABJACK/CMD?name=PULSE&npulse=2";
	  std::cout<<sq.str()<<std::endl;
	  CurlQuery((char*) sq.str().c_str(),RC);
	  std::cout<<std::string(RC)<<std::endl;
	  ::sleep(1);
	}
      else
	{
	  _firstEvent[0]=_firstEvent[1]=0;
	  _currentGTC[0]=_currentGTC[1]=0;
	}
      //
      //sleep((uint32_t) 5);
      std::cout<<"======================> First event passed Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" DAC "<<dac<<std::endl;
      int nallow=ngtc+2;
      while (((_currentGTC[0]-_firstEvent[0])<ngtc || (_currentGTC[1]-_firstEvent[1])<ngtc )  && _loop)
	{usleep(100000);
	  std::cout<<"Waiting ======================>  event passed Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" for VthTime "<<dac<<" and ngtc"<<ngtc<<std::endl;
	  std::stringstream sq;sq<<"http://lyoac29:54000/LABJACK/CMD?name=PULSE&npulse="<<(int) nallow;
	  std::cout<<sq.str()<<std::endl;
	  CurlQuery((char*) sq.str().c_str(),RC);
	  std::cout<<std::string(RC)<<std::endl;
	  ::sleep(5);
	  int nl=0;
	  while ((_currentGTC[0]-_firstEvent[0])<ngtc ) {usleep(20000);std::cout<<_currentGTC[0]-_firstEvent[0]<<std::endl;nl++; if (nl>10) break;}
	  nl=0;
	  while ((_currentGTC[1]-_firstEvent[1])<ngtc ) {usleep(20000);std::cout<<_currentGTC[1]-_firstEvent[1]<<std::endl;nl++;if (nl>10) break;}
	  nallow=2;
	}
      std::cout<<"======================> End loop Current GTC "<<_currentGTC[0]<<" "<<_currentGTC[1]<<" VthTime "<<dac<<std::endl;
      for (int i=0;i<10;i++) {this->startAcquisition(false);usleep(10000);}

      dac-=thstep;
    }
}


void TdcManager::createTrees(std::string s)
{
  std::cout << " create Trees"<<std::endl;
  // Opening binary file
   std::stringstream filename("");    
  char dateStr [64];
            
  time_t tm= time(NULL);
  strftime(dateStr,20,"SMM_%d%m%y_%H%M%S",localtime(&tm));
  filename<<_directory<<"/"<<dateStr<<"_"<<_run<<".dat";
  _fdOut= ::open(filename.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  if (_fdOut<0)
    {
      perror("No way to store to file :");
      //std::cout<<" No way to store to file"<<std::endl;
      return;
    }  
  //

}
void TdcManager::closeTrees()
{
  ::close(_fdOut);
  

}

void TdcManager::process(std::string fullpath)
{
  fflush(stdout);
  //return;
 int fd=::open(fullpath.c_str(),O_RDONLY);
 uint8_t buf[0x20000];
 uint16_t* sbuf=(uint16_t*) buf;
 uint32_t* lbuf=(uint32_t*) buf;
 
  if (fd<0) 
    {
      printf("%s  Cannot open file %s : return code %d \n",__PRETTY_FUNCTION__,fullpath.c_str(),fd);
      //LOG4CXX_FATAL(_logShm," Cannot open shm file "<<fname);
      return ;
    }
  int info[4];
  info[0]=_run;
  info[1]=_eventStruct.mezzanine;
  if (_eventStruct.mezzanine ==1)
    {_eventStruct.idx++;info[2]=_eventStruct.idx;}
  else
    {_eventStruct.event++;info[2]=_eventStruct.event;}
  _eventStruct.abcid=0;
  _eventStruct.time=time(0)-_t0;
  memset(buf,0,0x20000);
  int size_buf=::read(fd,buf,0x20000);
  info[3]=size_buf;
  // printf(" Reading files with %d bytes \n",size_buf);
  // for (int i=0;i<64;i++)
  //    {
     
  //  	printf("%.2x ",(uint8_t) buf[i]);
    
  //   }
  // printf("\n");
  // getchar();
  
 
  #ifdef OLDFIRMWARE
  uint32_t nlines=ntohs(sbuf[4]);
  // printf("\n #lines %d %x %x \n ",nlines,lbuf[0],ntohl(lbuf[0]));
  uint64_t ll=buf[19]|((uint64_t) buf[18]<<8)|((uint64_t) buf[17]<<16)|((uint64_t) buf[16]<<24)|((uint64_t) buf[15]<<32)|((uint64_t)buf[14]<<40);;
  
  //printf("Event %d TDC %d ABCID %lx  time %f %d %s \n",_eventStruct.idx,_eventStruct.mezzanine,(ll),ll*2E-7,_eventStruct.event,fullpath.c_str());
  //getchar();
  _eventStruct.abcid=ll;

  /// pas de loop

  for (int i=1;i<nlines;i++)
    {
     
      uint8_t *fr=&buf[10+i*10];
      
      // it=fr[3]|(fr[4]<<8)|(fr[5]<<16)|(fr[6]<<24)|(fr[7]<<32);
      // double rti=it*2.5/256;
      // fprintf(fp,"%d,%llx,%f\n",ch,it,rti);
      // printf("%x,%d,%d,%llx,%f,%llx\n",fr[2],i,ch,it,rti,(fr[7]<<32));
      _eventStruct.strip=(fr[0]&0XFF);
      _eventStruct.bcid=fr[1]|(fr[2]<<8);
      _eventStruct.ltdc=fr[9]|((uint64_t)fr[8]<<8)|((uint64_t)fr[7]<<16)|((uint64_t)fr[6]<<24)|((uint64_t) fr[5]<<32);
      uint64_t coarse =((uint64_t)fr[8])|((uint64_t)fr[7]<<8)|((uint64_t)fr[6]<<16)|((uint64_t) fr[5]<<24);
      //if (_eventStruct.strip==8) printf("TDC %d COARSE %x \n", _eventStruct.mezzanine,coarse);
      _eventStruct.ltdc=coarse*1.0;
      _eventStruct.rtdc=fr[9]*2.5/256+2.5*coarse;
      //printf("%x,%d,%d,%llx,%f,%llx\n",fr[2],i,ch,it,rti,(fr[7]<<32));
      //  _tEvents->Fill();
      
    }
  #else
  #ifdef OLDSOFTWARE
  uint8_t ll=8;

  _currentABCID[_eventStruct.mezzanine-1]=buf[ll+ll-1]|((uint64_t) buf[ll+ll-2]<<8)|((uint64_t) buf[ll+ll-3]<<16)|((uint64_t) buf[ll+ll-4]<<24)|((uint64_t) buf[ll+ll-5]<<32)|((uint64_t)buf[ll+ll-6]<<40);
  _currentGTC[_eventStruct.mezzanine-1]= buf[ll+1]|((uint32_t) buf[ll]<<8);

  //printf("New buffer %d mezzanine %d \n",_currentGTC[_eventStruct.mezzanine-1],_eventStruct.mezzanine);
  _firstEvent[_eventStruct.mezzanine-1]=_firstEvent[_eventStruct.mezzanine-1] || (_currentGTC[_eventStruct.mezzanine-1]==1);
  #else
   uint32_t* itemp=(uint32_t*) buf;
  uint64_t* ltemp=(uint64_t*) buf;
  
  int m=itemp[4];
  _currentABCID[m-1]=ltemp[1];
  _currentGTC[m-1]=itemp[1];

  // for (int i=0;i<24;i++)
  //  {
     
  //   	printf("%.2x ",(uint8_t) buf[i]);
    
  //  }
  printf("New buffer %d mezzanine %d/%d GTC %d abdcid %f from %x %d chans \n",_currentGTC[_eventStruct.mezzanine-1],_eventStruct.mezzanine-1,m,itemp[1],ltemp[1]*2E-7,itemp[5],itemp[6]);
  
  if ((!_firstEvent[m-1]) && (_currentGTC[m-1]==0)) _firstEvent[m-1] =_currentGTC[m-1];
  
  #endif
  #endif
  //printf("%d bytes read %x %d \n",size_buf,cbuf[0],cbuf[1]);
  int ier=write(_fdOut,info,4*sizeof(uint32_t));
  ier=write(_fdOut,buf,size_buf);
 remove:
  ::close(fd);
  ::unlink(fullpath.c_str());

 fflush(stdout);
}

void TdcManager::rm(std::string fullpath)
{
  ::unlink(fullpath.c_str());

}

void TdcManager::ls(std::string sourcedir,std::vector<std::string>& res)
{
 
  res.clear();
  int count,i;  
  struct direct **files;  
  
  count = scandir(sourcedir.c_str(), &files, file_select_tdcmanager, alphasort);          
  /* If no files found, make a non-selectable menu item */  
  if(count <= 0)    {return ;}
       
  std::stringstream sc;         
  //printf("Number of files = %d\n",count);  
  for (i=1; i<count+1; ++i)  
    {
      // file name
      sc.str(std::string());
      sc<<sourcedir<<"/"<<files[i-1]->d_name;
      res.push_back(sc.str());
       
      /* sc.str(std::string());
       sd.str(std::string());
       sc<<sourcedir<<"/closed/"<<files[i-1]->d_name;
       sd<<sourcedir<<"/"<<files[i-1]->d_name;
       ::unlink(sc.str().c_str());
       ::unlink(sd.str().c_str());
      */
       free(files[i-1]);
    }
  free(files);
  return;
}

void TdcManager::doStart()
{
  
  // data directory

  std::stringstream sdir1,sdir2;
  sdir1<<"/dev/shm/"<<_sTDC1->hostTo()<<"/"<<_sTDC1->portTo();
  #ifdef USE_TDC2
  sdir2<<"/dev/shm/"<<_sTDC2->hostTo()<<"/"<<_sTDC2->portTo();
  #endif
  // List files on TDC1 and TDC2 and remove them
  std::vector<std::string> lfiles;
  this->ls(sdir1.str(),lfiles);
  for (  std::vector<std::string>::iterator it=lfiles.begin();it!=lfiles.end();it++) this->rm((*it));
  this->ls(sdir2.str(),lfiles);
  for (  std::vector<std::string>::iterator it=lfiles.begin();it!=lfiles.end();it++) this->rm((*it));

  // Initial time
  _t0=time(0);

 
  while(_running || true)
    {
      // Liste files for TDC1
  this->ls(sdir1.str(),lfiles);
  _eventStruct.mezzanine=1;
  for (  std::vector<std::string>::iterator it=lfiles.begin();it!=lfiles.end();it++) {
    //  std::cout<<" Processing "<<(*it)<<std::endl;
    this->process((*it));
    // std::cout<<" Processed "<<(*it)<<std::endl;
  }
  this->ls(sdir2.str(),lfiles);
  _eventStruct.mezzanine=2;
  for (  std::vector<std::string>::iterator it=lfiles.begin();it!=lfiles.end();it++) {
    //  std::cout<<" Processing "<<(*it)<<std::endl;
    this->process((*it));
    // std::cout<<" Processed "<<(*it)<<std::endl;
  }
      
  ::usleep(100);
    }

    
}


void TdcManager::start(levbdim::fsmmessage* m)
{
  //LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  for (std::map<uint64_t,uint32_t>::iterator idr=_msh->readoutMap().begin();idr!=_msh->readoutMap().end();idr++)
    idr->second=0;

  // Create run file
  Json::Value jc=m->content();
  _run=jc["run"].asInt();
  _type=jc["type"].asInt();
  if (m->content().isMember("directory"))
    {
      _directory=jc["directory"].asString();
      this->parameters()["directory"]=jc["directory"];
    }
  else
    if (this->parameters().isMember("directory"))
      {
	 _directory=this->parameters()["directory"].asString();
      }
    else
      _directory="/tmp";
  std::stringstream s;
  s<<_directory<<"/tdc"<<_run<<".root";
  
  this->createTrees(s.str());
  // Clear evnt number
   for (uint32_t i=0;i<2;i++)
    {
      if (_msh->tdc(i)==NULL) continue;
      _msh->tdc(i)->clear();
    }
 
  switch (_type)
    {
    case 0:
      {
      this->startAcquisition(true);
      break;
      }
    case 1:
      {
      _loop=true;
      uint8_t dacmin=jc["dacmin"].asInt() & 0xFF;
      uint8_t dacmax=jc["dacmax"].asInt() & 0xFF;
      uint8_t dacstep=jc["dacstep"].asInt() & 0xFF;
      uint8_t ngtc=jc["ngtc"].asInt() & 0xFF;
      this->loop6BDac(dacmin,dacmax,dacstep,ngtc);
      break;
      }
    case 2:
      {
      _loop=true;
      uint32_t thmin=jc["thmin"].asInt() & 0xFFFF;
      uint32_t thmax=jc["thmax"].asInt() & 0xFFFF;
      uint32_t thstep=jc["thstep"].asInt() & 0xFFFF;
      uint8_t ngtcv=jc["ngtc"].asInt() & 0xFFFF;
      this->loopVthTime(thmin,thmax,thstep,ngtcv);
      break;
      }
    }
}
void TdcManager::stop(levbdim::fsmmessage* m)
{
  //LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  
  this->startAcquisition(false);
  ::sleep(2);
  //g_run.join_all();
  this->closeTrees();
}
void TdcManager::destroy(levbdim::fsmmessage* m)
{
  _running=false;
  g_run.join_all();
  //LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  delete _group;
  delete _sCtrl;
  delete _sTDC1;
  #ifdef USE_TDC2
  delete _sTDC2;
  #endif
  delete _msh;
  
}
void TdcManager::parseConfig(std::string name)
{
  _slcBytes=0;
  memset(_slcBuffer,0,0x1000*sizeof(uint16_t));
  memset(_slcAddr,0,0x1000*sizeof(uint16_t));
  
  FILE *fp=fopen(name.c_str(), "r");
  int ier;
  do
  {
    ier=fscanf(fp,"%x",&_slcBuffer[_slcBytes]);
    _slcAddr[_slcBytes]=_slcBytes;
    _slcBytes++;
  } while (ier!=EOF && _slcBytes<0x1000);
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;
  fclose(fp);
}

void TdcManager::writeRamAvm()
{
  if (_slcBuffer[1]<2) return;
  uint16_t sockbuf[0x20000];
  sockbuf[0]=htons(0xFF00);
  sockbuf[1]=htons(_slcBytes);
  int idx=2;
  for (int i=0;i<_slcBytes;i++)
  {
    sockbuf[idx]=htons(_slcAddr[i]);
    sockbuf[idx+1]=htons(_slcBuffer[i]);
    idx+=2;
  }
  // Send the Buffer
  try
  {
    _sCtrl->send((const void*) sockbuf,idx*sizeof(uint16_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
  
}
void TdcManager::queryCRC()
{
  _slcBuffer[0]=0x208;
  _slcAddr[0]=0x1;
  _slcBytes=1;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
   _slcBytes++;
  this->writeRamAvm();
  
}
void TdcManager::startAcquisition( bool start)
{

 uint16_t sockbuf[0x200];
 sockbuf[0]=htons(0xFF00);
 sockbuf[1]=htons(1);
 sockbuf[2]=htons(0x220);
 if (start)
   sockbuf[3]=htons(1);
 else
   sockbuf[3]=htons(0);
  // Send the Buffer
  try
  {
    _sCtrl->send((const void*) sockbuf,4*sizeof(uint16_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }




  
  
}
