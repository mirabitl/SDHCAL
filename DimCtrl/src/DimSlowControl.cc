

#include "DimSlowControl.h"
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

#include <string.h>



DimSlowControl::DimSlowControl() : _storeRunning(false),_checkRunning(false),_my(NULL)
{
  _chambers.clear();
  DimBrowser* dbr=new DimBrowser(); 
  char *service, *format; 
  int type;
  // Get DB service
  cout<<"On rentre dans scandns "<<endl;

  char *server,*node;
  _BMPPrefix="";
  dbr->getServices("/BMP183/*/PRESSIONREADVALUES" ); 
  while(type = dbr->getNextService(service, format)) 
    { 
      cout << service << " -  " << format << endl; 
      std::string ss;
      ss.assign(service);
      size_t n=ss.find("/PRESSIONREADVALUES");
      _BMPPrefix=ss.substr(0,n);
    }
  // Register Info
  _pressionInfo=NULL;
  _temperatureInfo=NULL;
  if (_BMPPrefix.length()>6)
    {
      std::stringstream s0;
      s0.str(std::string());
      s0<<_BMPPrefix<<"/PRESSIONREADVALUES";
      _pressionInfo= new DimInfo(s0.str().c_str(),_PRead,this);
      s0.str(std::string());
      s0<<_BMPPrefix<<"/TEMPERATUREREADVALUES";
      _temperatureInfo= new DimInfo(s0.str().c_str(),_TRead,this);
    }
  else
    LOG4CXX_FATAL(_logCtrl,"NO P,T readout ");

  for (int i=0;i<7;i++)
    for (int j=0;j<8;j++)
      {
	std::stringstream s0;
	s0.str(std::string());
	s0<<"/WIENER/MODULE"<<i<<"-CHANNEL"<<j<<"/"<<i*8+j;
	_wienerInfo[i*8+j]=new DimInfo(s0.str().c_str(),&_hvchannels[i*8+j],sizeof(wienerChannel),this);
      }
      
}
DimSlowControl::~DimSlowControl()
{
 
}

void DimSlowControl::infoHandler()
{
   DimInfo *curr = getInfo(); // get current DimInfo address 
   //if (curr->getSize()==1) return;
   if (curr==_pressionInfo)
     {
       _PRead=curr->getFloat();
       return;
     }
   if (curr==_temperatureInfo)
     {
       _TRead=curr->getFloat();
       return;
     }
   for (int i=0;i<56;i++)
     
     if (curr==_wienerInfo[i])
       {
	 memcpy(&_hvchannels[i],curr->getData(),curr->getSize());
	 return;
       }

}

void DimSlowControl::setReadoutPeriod(uint32_t p)
{
  std::stringstream s0;
  if (_BMPPrefix.length()>3)
    {
      s0.str(std::string());
      s0<<_BMPPrefix<<"/SETPERIOD";
      DimClient::sendCommand(s0.str().c_str(),(int) p);
    }
  
  DimClient::sendCommand("/WIENER/SetPeriod",(int) p);
}

void DimSlowControl::readChannel(uint32_t ch)
{
  DimClient::sendCommand("/WIENER/ReadChannel",(int) ch);
}
void DimSlowControl::setVoltage(uint32_t ch,float v)
{
  std::stringstream s0;
  s0.str(std::string());
  int32_t ibuf[2];
  float* fbuf=(float*) ibuf;
  ibuf[0]=ch;fbuf[1]=v;
  DimClient::sendCommand("/WIENER/SetPeriod",ibuf,sizeof(int32_t)+sizeof(float));
 
}
void DimSlowControl::setCurrentLimit(uint32_t ch,float v)
{
  std::stringstream s0;
  s0.str(std::string());
  int32_t ibuf[2];
  float* fbuf=(float*) ibuf;
  ibuf[0]=ch;fbuf[1]=v;
  DimClient::sendCommand("/WIENER/SetCurrent",ibuf,sizeof(int32_t)+sizeof(float));
 
}
void DimSlowControl::HVON(uint32_t ch)
{
  std::stringstream s0;
  s0.str(std::string());
  int32_t ibuf[2];
  ibuf[0]=ch;ibuf[1]=1;
  DimClient::sendCommand("/WIENER/Switch",ibuf,2*sizeof(int32_t));
 
}
void DimSlowControl::HVOFF(uint32_t ch)
{
  std::stringstream s0;
  s0.str(std::string());
  int32_t ibuf[2];
  ibuf[0]=ch;ibuf[1]=0;
  DimClient::sendCommand("/WIENER/Switch",ibuf,2*sizeof(int32_t));
 
}




void DimSlowControl::initialiseDB(std::string s)
{

  _my= new MyInterface(s);
  _my->connect();
  _my->executeQuery("select HVCHAN,VREF,P0,T0 FROM CHAMBERREF WHERE FIN>NOW() AND DEBUT<NOW()");
  MYSQL_ROW row=NULL;
  while ((row=_my->getNextRow())!=0)
    {
      chamberRef c;
      c.channel=atoi(row[0]);
      c.vref=atof(row[1]);
      c.iref=atof(row[2]);
      c.p0=atof(row[3]);
      c.t0=atof(row[4]);
      _chambers.push_back(c);
    }
  _my->disconnect();

}

void DimSlowControl::loadReferences()
{
  for (std::vector<chamberRef>::iterator it=_chambers.begin();it!=_chambers.end();it++)
    {
      LOG4CXX_INFO(_logCtrl,"Setting "<<it->channel<<" to V="<<it->vref<<" Imax="<<it->iref);
      this->setVoltage(it->channel,it->vref);
      this->setCurrentLimit(it->channel,it->iref);
    }
}
bool  DimSlowControl::getPTMean()
{
  if (_my==NULL)
    {
      LOG4CXX_ERROR(_logCtrl,"No mysql interface available");
      return false;
    }
  _my->connect();
  _my->executeQuery("select P,TK FROM BMPMON WHERE TIS BETWEEN NOW() - INTERVAL 10 MINUTE AND NOW()");
  _PMean=0;_TMean=0;
  int32_t nmeas=0;
  MYSQL_ROW row=NULL;
  while ((row=_my->getNextRow())!=0)
    {
      _PMean+=atof(row[0]);
      _TMean+=atof(row[1]);
      nmeas++;
    }
  _my->disconnect();
  if (nmeas<5)
    {
      LOG4CXX_WARN(_logCtrl,"Not enough PT measure");
      return false;
      
    }
  _PMean/=nmeas;
  _TMean/=nmeas;
  return true;
  
}

float DimSlowControl::vset(uint32_t i){ return _hvchannels[i].vset;}
float DimSlowControl::iset(uint32_t i){ return _hvchannels[i].iset;}
float DimSlowControl::vout(uint32_t i){ return _hvchannels[i].vout;}
float DimSlowControl::iout(uint32_t i){ return _hvchannels[i].iout;}


std::string DimSlowControl::hvinfoChannel(uint32_t ch)
{
  Json::FastWriter fastWriter;
  Json::Value fromScratch;

  fromScratch["channel"] = _hvchannels[ch].channel;
  fromScratch["vset"] = _hvchannels[ch].vset;
  fromScratch["iset"] = _hvchannels[ch].iset;
  fromScratch["vout"] = _hvchannels[ch].vout;
  fromScratch["iout"] = _hvchannels[ch].iout;
  return fastWriter.write(fromScratch);
}
std::string DimSlowControl::hvinfoCrate()
{
  Json::FastWriter fastWriter;

  Json::Value array;

  for (int ch=0;ch<56;ch++)
      {
	Json::Value fromScratch;
	fromScratch["channel"] = _hvchannels[ch].channel;
	fromScratch["vset"] = _hvchannels[ch].vset;
	fromScratch["iset"] = _hvchannels[ch].iset;
	fromScratch["vout"] = _hvchannels[ch].vout;
	fromScratch["iout"] = _hvchannels[ch].iout;

	array.append(fromScratch);
      }
    return fastWriter.write(array);
}


void DimSlowControl::startStore(uint32_t tempo)
{
  _storeTempo=tempo;
  g_store.create_thread(boost::bind(&DimSlowControl::doStore, this));
	
}
void DimSlowControl::doStore()
{
  _storeRunning=true;
  LOG4CXX_INFO(_logCtrl,"Storage thread started");
  while (_storeRunning)
    {
      if (_my==NULL)
	{
	  sleep((unsigned int) 10);
	  continue;
	}
      _my->connect();
      for (int i=0;i<56;i++)
	{
	  std::stringstream s0;
	  s0.str(std::string());
	  s0<<"insert into WIENERMON(HVCHAN,VSET,ISET,VOUT,IOUT) VALUES("<<_hvchannels[i].channel<<","<<_hvchannels[i].vset<<","<<_hvchannels[i].iset<<","<<_hvchannels[i].vout<<","<<_hvchannels[i].iout<<")";
	  _my->executeQuery(s0.str());

	}
      _my->disconnect();
      sleep((unsigned int) _storeTempo);
    }
  LOG4CXX_INFO(_logCtrl,"Storage thread stopped");
}


void DimSlowControl::stopStore()
{
  _storeRunning=false;
  g_store.join_all();
  LOG4CXX_INFO(_logCtrl,"Storage thread destroy");
}
void DimSlowControl::startCheck(uint32_t tempo)
{
  _checkTempo=tempo;
  g_check.create_thread(boost::bind(&DimSlowControl::doCheck, this));
	
}
void DimSlowControl::doCheck()
{
  _checkRunning=true;
  LOG4CXX_INFO(_logCtrl,"Check thread started");
  while (_checkRunning)
    {
      if (!this->getPTMean())
	{
	  sleep((unsigned int) 10);
	  continue;
	}
      _my->connect();
      for (std::vector<chamberRef>::iterator it=_chambers.begin();it!=_chambers.end();it++)
	{
	  std::stringstream s0;
	  s0.str(std::string());
	  s0<<"select VSET,VOUT FROM WIENERMON WHERE  HVCHAN="<<it->channel<<" ORDER BY IDX DESC LIMIT 1";
	  _my->executeQuery(s0.str());
	  MYSQL_ROW row=NULL;
	  float vset=0,vout=0;
	  while ((row=_my->getNextRow())!=0)
	    {
	      vset=atof(row[0]);
	      vout=atof(row[1]);
	    }
 
	  float vexpected=it->vref*it->t0/it->p0*_PMean/_TMean;
	  float veffective=vout*it->p0/it->t0*_TMean/_PMean;
	  float deltav=abs(veffective-it->vref);
	  if (deltav>10 and deltav<200)
	    {
	      LOG4CXX_INFO(_logCtrl,"checkChannel "<<it->channel<<" : Vout"<<vout<<" Veffective "<<veffective<<" Vref "<<it->vref<<" , the expected value "<<vexpected<<" will be set automatically ");
	      this->setVoltage(it->channel,vexpected);
	      this->readChannel(it->channel);
	      std::cout<<hvinfoChannel(it->channel);
	    }
	  if (deltav>=200 && vout>=5000)
	    LOG4CXX_WARN(_logCtrl,"checkChannel "<<it->channel<<" : Vout"<<vout<<" Veffective "<<veffective<<" Vref "<<it->vref<<" , the expected value "<<vexpected<<" cannot be set automatically  Delta= "<<deltav);
	  
	  if (deltav>=200 && vout<500)
	    LOG4CXX_WARN(_logCtrl,"checkChannel " <<it->channel<<" : Vout "<<vout<<" the channel is OFF");


	}
      _my->disconnect();
      sleep((unsigned int) _checkTempo);
    }
  LOG4CXX_INFO(_logCtrl,"Check thread stopped");
}


void DimSlowControl::stopCheck()
{
  _checkRunning=false;
  g_check.join_all();
  LOG4CXX_INFO(_logCtrl,"Check thread destroy");
}


