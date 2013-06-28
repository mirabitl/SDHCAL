
#include "CCCClient.h"
using namespace std;
CCCClient::CCCClient(std::string host,uint32_t port) : NMClient(host,port)
  {
   
    //this->start();
    nessai_=0;
    nbytes_=0;
    //bsem_.lock();
  }
  
  NetMessage* CCCClient::serviceHandler(NetMessage* m)
  {
    nessai_++;
    nbytes_+=m->getPayloadSize()*1.;
    if (nessai_%1000 ==0)
{
      uint32_t* ibuf=(uint32_t*) m->getPayload();
      printf(" J'ai recu un service %s %d %d-%d %f \n",m->getName().c_str(),m->getPayloadSize(),nessai_,ibuf[0],nbytes_/1024./1024.);
}
    return NULL;
  }
  void CCCClient::sendSecureCommand(std::string s,NetMessage* m)
  {
    // printf("LOCKING %s\n",s.c_str());
    //bsem_.lock();
    this->sendCommand(s,m);
    // printf("LOCKING %s\n",s.c_str());
    bsem_.lock();
    //bsem_.unlock();

  }

  NetMessage* CCCClient::answerHandler(NetMessage* m)
  {
    printf("I got answer for command %s \n",m->getName().c_str());
    aSize_=m->getPayloadSize();
    memcpy(aBuf_,m->getPayload(),aSize_);
    //sleep((unsigned int) 5);
    bsem_.unlock();
    //global_stream_lock.unlock();
    return NULL;
  }


void CCCClient::doRegistration()
{
  this->registerHandler("INTIALISE",boost::bind(&CCCClient::answerHandler,this,_1));
  this->registerHandler("CONFIGURE",boost::bind(&CCCClient::answerHandler,this,_1));
  this->registerHandler("START",boost::bind(&CCCClient::answerHandler,this,_1));
  this->registerHandler("STOP",boost::bind(&CCCClient::answerHandler,this,_1));
  this->registerHandler("PAUSE",boost::bind(&CCCClient::answerHandler,this,_1));
  this->registerHandler("RESUME",boost::bind(&CCCClient::answerHandler,this,_1));
  this->registerHandler("DIFRESET",boost::bind(&CCCClient::answerHandler,this,_1));
  this->registerHandler("CCCRESET",boost::bind(&CCCClient::answerHandler,this,_1));
  this->registerHandler("TESTREGISTERWRITE",boost::bind(&CCCClient::answerHandler,this,_1));
  this->registerHandler("TESTREGISTERREAD",boost::bind(&CCCClient::answerHandler,this,_1));
  this->start();
}
void CCCClient::doInitialise(std::string dcc)
{
  NetMessage m("INITIALISE",NetMessage::COMMAND,dcc.size());
  m.setPayload((unsigned char*)dcc.c_str(),dcc.size());
  printf("Payload size is %d %d \n",dcc.size(),m.getPayloadSize());
  this->sendSecureCommand("INITIALISE",&m);
}

void CCCClient::doConfigure()
{
   NetMessage m("CONFIGURE",NetMessage::COMMAND,4);
   this->sendCommand("CONFIGURE",&m);
}
void CCCClient::doStart()
{
   NetMessage m("START",NetMessage::COMMAND,4);
   this->sendSecureCommand("START",&m);
}
void CCCClient::doStop()
{
   NetMessage m("STOP",NetMessage::COMMAND,4);
   this->sendSecureCommand("STOP",&m);
}

void CCCClient::doPause()
{
   NetMessage m("PAUSE",NetMessage::COMMAND,4);
   this->sendCommand("PAUSE",&m);
}

void CCCClient::doResume()
{
   NetMessage m("RESUME",NetMessage::COMMAND,4);
   this->sendCommand("RESUME",&m);
}

void CCCClient::doDIFReset()
{
   NetMessage m("DIFRESET",NetMessage::COMMAND,4);
   this->sendCommand("DIFRESET",&m);
}

void CCCClient::doCCCReset()
{
   NetMessage m("CCCRESET",NetMessage::COMMAND,4);
   this->sendCommand("CCCRESET",&m);
}

void CCCClient::doTestRegisterWrite(uint32_t r)
{
 uint32_t reg=r;
 NetMessage m("TESTREGISTERWRITE",NetMessage::COMMAND,sizeof(uint32_t));
 m.setPayload((unsigned char*)&reg,sizeof(uint32_t));
 this->sendSecureCommand("TESTREGISTERWRITE",&m);
}
uint32_t CCCClient::doTestRegisterRead()
{
   NetMessage m("TESTREGISTERREAD",NetMessage::COMMAND,4);
   this->sendSecureCommand("TESTREGISTERREAD",&m);
   uint32_t reg=0;
   memcpy(&reg,aBuf_,sizeof(uint32_t));
   printf("return code is %x \n",reg);
   return reg;
   
}

