/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <json/writer.h> 
#include "json/json.h"
#include <iostream>
#include <fstream>
#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WMenuItem>
#include <Wt/WTabWidget>
#include <Wt/WTextArea>
#include <Wt/WGroupBox>
#include <Wt/WCheckBox>
#include <Wt/WSpinBox>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WEnvironment>
#include <Wt/WTable>

// c++0x only, for std::bind
// #include <functional>
using namespace  std;
#include "LCIOWritterInterface.h"
#include "DIFMultiClient.h"
#include "OracleDIFDBManager.h"
#include "Tokenizer.h"
#include <json/json.h>
//using namespace Wt;

/*
 * A simple hello world application class which demonstrates how to react
 * to events, read input, and give feed-back.
 */
class levbc : public Wt::WApplication
{
public:
  levbc(const Wt::WEnvironment& env);

private:
  DIFMultiClient* theClient_;
  ShmProxy* theProxy_;
  OracleDIFDBManager* theDBManager_;
  RunInfo* theRunInfo_;
  uint32_t theRunNumber_;


  Wt::WLineEdit *LE_CCCHost_;
  Wt::WLineEdit *LE_DIFHosts_;
  Wt::WLineEdit *LE_DataDirectory_;
  Wt::WLineEdit *LE_DBState_;
  Wt::WLineEdit *LE_ControlRegister_;
  Wt::WLineEdit *LE_Session_;
  Wt::WCheckBox *CB_SaveData_;
  Wt::WSpinBox  *SB_CCCPort_;
  Wt::WSpinBox  *SB_DIFPort_;
  Wt::WSpinBox  *SB_NumberOfDIFs_;
  Wt::WSpinBox  *SB_DefaultRunNumber_;
  Wt::WLineEdit *LE_State_;
  Wt::WPushButton *PB_Initialise_;
  Wt::WPushButton *PB_Configure_;
  Wt::WPushButton *PB_Start_;
  Wt::WPushButton *PB_Stop_;
  Wt::WPushButton *PB_Halt_;
  Wt::WPushButton *PB_Destroy_;
  Wt::WPushButton *PB_RefreshStatus_;
  Wt::WTable *TB_DIFStatus_;
  Wt::WTable *TB_TrigStatus_;
  Wt::WLineEdit *LE_RunStatus_;
  Wt::WLineEdit *LE_RunNumber_;
  Wt::WLineEdit *LE_EventNumber_;
  Wt::WContainerWidget *container_;
  Wt::WTabWidget *tabW_; 
  

  void Quit();
  void buildDaqForm(Wt::WGroupBox *wb);
  void InitialiseButtonHandler();
  void InitialiseAction();
  void ConfigureButtonHandler();
  void ConfigureAction();

  void StartButtonHandler();
  void StartAction();
  void StopButtonHandler();
  void StopAction();
  void HaltButtonHandler();
  void HaltAction();
  void DestroyButtonHandler();
  void DestroyAction();

  void createMonitoringForm(Wt::WGroupBox *wb);
  
  void buildMonitoringForm(Wt::WGroupBox *wb);
  void updateMonitoringForm();

};





/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.

DBSTATE AllInOne

Wt::WContainerWidget *container = new Wt::WContainerWidget();


*/
std::string exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}
levbc::levbc(const Wt::WEnvironment& env)
  : Wt::WApplication(env),theClient_(NULL),LE_RunStatus_(NULL),TB_DIFStatus_(NULL),LE_RunNumber_(NULL),LE_EventNumber_(NULL)
{

   std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-state -v");
   std::cout<<"Youpi Ya "<<res<<std::endl;
  setTitle("Standalone Data Acquisition");                               // application title
  container_ = new Wt::WContainerWidget(root());

  tabW_ = new Wt::WTabWidget(container_);
 

  Wt::WGroupBox *groupBoxDaq = new Wt::WGroupBox("Daq Control");
  tabW_->addTab(groupBoxDaq,"Daq Control");//, Wt::WTabWidget::PreLoading);

  groupBoxDaq->addStyleClass("centered-example");

  buildDaqForm(groupBoxDaq);
  Wt::WGroupBox *groupBoxDaqStatus = new Wt::WGroupBox("Daq Status");
  tabW_->addTab(groupBoxDaqStatus,"Daq Status");//, Wt::WTabWidget::PreLoading);

  groupBoxDaqStatus->addStyleClass("centered-example");

  buildMonitoringForm(groupBoxDaqStatus);

  tabW_->setStyleClass("tabwidget");

  root()->addWidget(new Wt::WBreak());                       // insert a line break
  Wt::WPushButton *b1 = new Wt::WPushButton("Kill the Session", root()); // create a button
  b1->setMargin(5, Wt::Left);                                 // add 5 pixels margin

  root()->addWidget(new Wt::WBreak());                       // insert a line break



  /*
   * Connect signals with slots
   *
   * - simple Wt-way
   */

  b1->clicked().connect(this, &levbc::Quit);
  tabW_->currentChanged().connect(this,&levbc::updateMonitoringForm);

}

void levbc::Quit()
{
  /*
   * Update the text, using text input into the nameEdit_ field.
   */
  std::string sh="http://"+this->environment().hostName()+"/test.html";		
  this->redirect(sh);

  this->quit();
}
void levbc::buildDaqForm(Wt::WGroupBox *wb)
{

  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  //vbox->setContentsMargins(2,2,2,2);
  wb->setLayout(vbox);
  //wb->resize(200,200);

  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();



  PB_Initialise_= new Wt::WPushButton("Initialise", wb);
  PB_Configure_= new Wt::WPushButton("Configure", wb);//PB_Configure_->disable();
  PB_Start_= new Wt::WPushButton("Start", wb);//PB_Start_->disable();
  PB_Stop_= new Wt::WPushButton("Stop", wb);//PB_Stop_->disable();
  PB_Halt_= new Wt::WPushButton("Halt", wb);//PB_Halt_->disable();
  PB_Destroy_= new Wt::WPushButton("Destroy", wb);//PB_Destroy_->disable();
  hbFSM->addWidget(PB_Initialise_);  // show some text
  hbFSM->addWidget(PB_Configure_);  // show some text
  hbFSM->addWidget(PB_Start_);  // show some text
  hbFSM->addWidget(PB_Stop_);  // show some text
  hbFSM->addWidget(PB_Halt_);  // show some text
  hbFSM->addWidget(PB_Destroy_);  // show some text
  vbox->addLayout(hbFSM);
  LE_State_ = new Wt::WLineEdit(wb);                     // allow text input
  LE_State_->setText("Destroyed");
  LE_State_->disable();
  vbox->addWidget(LE_State_);
  PB_Initialise_->clicked().connect(this,&levbc::InitialiseButtonHandler);

  PB_Start_->clicked().connect(this,&levbc::StartButtonHandler);
  PB_Configure_->clicked().connect(this,&levbc::ConfigureButtonHandler);
  PB_Stop_->clicked().connect(this,&levbc::StopButtonHandler);
  PB_Halt_->clicked().connect(this,&levbc::HaltButtonHandler);
  PB_Destroy_->clicked().connect(this,&levbc::DestroyButtonHandler);


}

void levbc::InitialiseButtonHandler()
{
  try {
    this->InitialiseAction();
    LE_State_->setText("Initialised");
    PB_Configure_->enable();
  }
  catch(...)
    {
      LE_State_->setText("Failure not Initialised");
    }

  PB_Initialise_->disable();
  PB_Destroy_->enable();



}
void levbc::InitialiseAction()
{

  // Instantiate
  theClient_ = new DIFMultiClient();
  // Add CCC Client
  printf("Connecting to %s port %d \n",LE_CCCHost_->text().toUTF8().c_str(),SB_CCCPort_->value());
  theClient_->addCCC(LE_CCCHost_->text().toUTF8().c_str(),SB_CCCPort_->value());

  // Use tokenizer to find all DIF hosts
  Tokenizer difs(LE_DIFHosts_->text().toUTF8(),",");
  //Obtaining Number Of Tokens (Remaining)
 
  std::vector<std::string> hostv=difs.split();
  for (std::vector<std::string>::iterator it=hostv.begin();it!=hostv.end();it++)
    {
      theClient_->addClient((*it),SB_DIFPort_->value());
      
      printf("Connecting to %s port %d \n",(*it).c_str(),SB_DIFPort_->value());
	     
    }
  //Initialisation
  printf("1\n");
  theClient_->Destroy();
    printf("2\n");

  theClient_->ScanDevices();
 
  printf("3\n");

  theClient_->Initialise();


  printf("4\n");

  // DIFDB manager
  theDBManager_= new OracleDIFDBManager("74",LE_DBState_->text().toUTF8());
  printf("5\n");

  theDBManager_->initialize();
  printf("6\n");

  theDBManager_->download();
   printf("7\n");

   if (CB_SaveData_->checkState())
    {
      LCIOWritterInterface* lc= new LCIOWritterInterface();
      theProxy_= new ShmProxy(SB_NumberOfDIFs_->value(),true,lc);
   
    }
  else
    theProxy_= new ShmProxy(SB_NumberOfDIFs_->value());
  

   std::cout<<"Fin de l'initialise"<<this->sessionId()<<std::endl;  
  theRunInfo_=NULL;
}
void levbc::ConfigureButtonHandler()
{
  try {
    this->ConfigureAction();
    LE_State_->setText("Configured");
    PB_Start_->enable();
  }
  catch(...)
    {
      LE_State_->setText("Failure not Configured");
    }

  PB_Configure_->disable();
  PB_Destroy_->enable();


}
void levbc::ConfigureAction()
{
   theProxy_->Initialise();
  printf("5\n");
  theProxy_->Configure();
  std::cout<<"On appelle CCC<anager Configure"<< LE_ControlRegister_->text().toUTF8().c_str()<<std::endl;
  uint32_t ctrlreg;sscanf((const char*) LE_ControlRegister_->text().toUTF8().c_str(),"0x%x",&ctrlreg);
  printf("CONTROL EGISTER========================================================================> %x \n",ctrlreg);
  theClient_->Configure(theDBManager_,ctrlreg);
  std::cout<<"Fin du configure"<<std::endl;
 //  std::vector<std::string> v=Daq::getListOfDaqNames();
//   for (std::vector<std::string>::iterator it=v.begin();it!=v.end();it++)
//     std::cout<<it->c_str()<<std::endl;
  if (theRunInfo_==NULL)
    {
      try {


	std::stringstream daqname("");    
	char dateStr [64];
            
	time_t tm= time(NULL);
	strftime(dateStr,50,"LaDaqAToto_%d%m%y_%H%M%S",localtime(&tm));
	daqname<<dateStr;
	Daq* me=new Daq(daqname.str());

	printf("la daq est creee %s\n",daqname.str().c_str());
	me->setStatus(0);
	printf("la daq a change de statut\n");
	me->setXML(".daq/last.txt");
	me->uploadToDatabase();
	printf("Upload DOne");
  
	theRunInfo_=new RunInfo(0,"LaDaqAToto");
	printf("le run est creee\n");
	theRunInfo_->setStatus(1);
	theRunNumber_=theRunInfo_->getRunNumber();
      } catch (ILCException::Exception e)
	{
	  theRunInfo_=NULL;
	  std::cout<<e.getMessage()<<std::endl;
	}
    }
  std::cout<<" New run starting ------------------->"<<theRunNumber_<<std::endl;
}
void levbc::StartButtonHandler()
{
  try {
    this->StartAction();
    LE_State_->setText("Running");
    PB_Stop_->enable();
    PB_Halt_->enable();
  }
  catch(...)
    {
      LE_State_->setText("Failure not Running");
    }

  PB_Start_->disable();
  PB_Destroy_->enable();

}
void levbc::StartAction()
{
  theClient_->getCCCClient()->doStart();
  theClient_->getCCCClient()->doStop();
  if (theRunInfo_== NULL)
    {
      theRunNumber_=20000;
    }
  else
    theRunInfo_->setStatus(2);

  std::cout<<" New run starting ------------------->"<<theRunNumber_<<std::endl;

  theProxy_->Start(theRunNumber_,LE_DataDirectory_->text().toUTF8());
  theClient_->Start();
  std::cout<<"Fin de l'Enable"<<std::endl;  

}
void levbc::StopButtonHandler()
{
  try {
    this->StopAction();
    LE_State_->setText("Stopped");
    PB_Start_->enable();
    PB_Halt_->disable();
  }
  catch(...)
    {
      LE_State_->setText("Failure not Stopped");
    }

  PB_Stop_->disable();
  PB_Destroy_->enable();

}
void levbc::StopAction()
{
  theClient_->Stop();
  theProxy_->Stop();
  if (theRunInfo_==NULL)
    theRunNumber_++;
  else
    {
      theRunInfo_->setStatus(4);
      delete theRunInfo_;
      theRunInfo_=new RunInfo(0,"LaDaqAToto");
      theRunInfo_->setStatus(1);
      theRunNumber_=theRunInfo_->getRunNumber();
      std::cout<<" New run starting ------------------->"<<theRunNumber_<<this->bookmarkUrl()<<std::endl;
    }
}

void levbc::HaltButtonHandler()
{
  try {
    this->HaltAction();
    LE_State_->setText("Halted");
    PB_Start_->enable();
    PB_Stop_->disable();
    PB_Configure_->enable();
  }
  catch(...)
    {
      LE_State_->setText("Failure not Halted");
    }

  PB_Halt_->disable();
  PB_Destroy_->enable();

}
void levbc::HaltAction()
{
  this->StopAction();
}

void levbc::DestroyButtonHandler()
{
  try {
    this->DestroyAction();
    LE_State_->setText("Destroyed");
    PB_Initialise_->enable();
    PB_Stop_->disable();
    PB_Start_->disable();
    PB_Halt_->disable();
    PB_Configure_->disable();
  }
  catch(...)
    {
      LE_State_->setText("Failure not Destroyed, Please quit");
    }


  PB_Destroy_->disable();

}

void levbc::DestroyAction()
{
   theClient_->Destroy();
   printf("2\n");
   delete theClient_;
   theClient_=NULL;
}

void levbc::createMonitoringForm(Wt::WGroupBox *wb)
{
 Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  //vbox->setContentsMargins(2,2,2,2);
  wb->setLayout(vbox);
  //wb->resize(200,200);

  Wt::WHBoxLayout *hbRun = new Wt::WHBoxLayout();
  
  if (LE_RunStatus_==NULL)
    {
      LE_RunStatus_ = new Wt::WLineEdit(wb);                     // allow text input
      LE_RunStatus_->disable();
      hbRun->addWidget(LE_RunStatus_);
    }
  if (LE_RunNumber_==NULL)
    {
	     
      LE_RunNumber_ = new Wt::WLineEdit(wb);                     // allow text input
      LE_RunNumber_->disable();
      hbRun->addWidget(LE_RunNumber_);
    }
  if (LE_EventNumber_==NULL)
    {
      LE_EventNumber_ = new Wt::WLineEdit(wb);                     // allow text input
      LE_EventNumber_->disable();
      hbRun->addWidget(LE_EventNumber_);
    }
  PB_RefreshStatus_= new Wt::WPushButton("Refresh", wb);
  PB_RefreshStatus_->clicked().connect(this, &levbc::updateMonitoringForm);
  hbRun->addWidget(PB_RefreshStatus_);
  vbox->addLayout(hbRun);
  Wt::WHBoxLayout *hbTrig = new Wt::WHBoxLayout();
  TB_TrigStatus_= new Wt::WTable(wb);
  TB_TrigStatus_->setHeaderCount(1);
  TB_TrigStatus_->setWidth("80%");
  TB_TrigStatus_->clear();
  TB_TrigStatus_->addStyleClass("table-bordered",true); 

  hbTrig->addWidget(TB_TrigStatus_);
  vbox->addLayout(hbTrig);
  Wt::WHBoxLayout *hbDIF = new Wt::WHBoxLayout();
      if (TB_DIFStatus_==NULL)
	{
	  printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
	  TB_DIFStatus_= new Wt::WTable(wb);
	  //printf("%s %d %x\n",__PRETTY_FUNCTION__,__LINE__,(uint32_t)TB_DIFStatus_);
	  TB_DIFStatus_->setHeaderCount(1);
	  printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
	  TB_DIFStatus_->setWidth("80%");
	  printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);

	  TB_DIFStatus_->clear();
	  printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
	  hbDIF->addWidget(TB_DIFStatus_);
	}
      vbox->addLayout(hbDIF);
      printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
}
void levbc::buildMonitoringForm(Wt::WGroupBox *wb)
{
  if (theClient_==NULL)
    {
      printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
      createMonitoringForm(wb);
      LE_RunStatus_->setText("DAQ is destroyed");
      

    }
  else
    {
      printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
      createMonitoringForm(wb);
      std::stringstream s("");
      s<<"Current Run is "<<theRunNumber_;
      std::cout<<s.str()<<std::endl;
      LE_RunStatus_->setText(s.str());
      printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
      updateMonitoringForm();
    }
}

void levbc::updateMonitoringForm()
{
  //if (theClient_==NULL) return;
  std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-state -v");
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(res,jsta);
  std::stringstream s;
  s<<jsta["stateResponse"]["stateResult"][0].asString();
  LE_RunStatus_->setText(s.str());
  //return;
  res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-evb -v");
  //std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  Json::Value jdev1;
  parsingSuccessful = reader.parse(jsta["shmStatusResponse"]["shmStatusResult"][0].asString(),jdev1);
  LE_RunNumber_->setText(jdev1["run"].asString());
  LE_EventNumber_->setText(jdev1["event"].asString());
  
res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-status -v");
  //std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  parsingSuccessful = reader.parse(jsta["statusResponse"]["statusResult"][0].asString(),jdev1);

  const Json::Value& jdevs= jdev1;
  //std::cout<<jdevs<<std::endl;
  //for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
  //  {	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
  //   std::cout<<(*jt);
  //  }
  // Header
  TB_DIFStatus_->clear();
  TB_DIFStatus_->setHeaderCount(1);
  printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
  TB_DIFStatus_->setWidth("80%");

  TB_DIFStatus_->elementAt(0, 0)->addWidget(new Wt::WText("Host"));
  TB_DIFStatus_->elementAt(0, 1)->addWidget(new Wt::WText("DIF"));
  TB_DIFStatus_->elementAt(0, 2)->addWidget(new Wt::WText("Status"));
  TB_DIFStatus_->elementAt(0, 3)->addWidget(new Wt::WText("L BCID"));
  TB_DIFStatus_->elementAt(0, 4)->addWidget(new Wt::WText("L GTC"));
  TB_DIFStatus_->elementAt(0, 5)->addWidget(new Wt::WText("Bytes"));

  uint32_t irow=1;
  for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
    {	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
      std::cout<<(*jt);
      new Wt::WText(Wt::WString::fromUTF8("oops"),TB_DIFStatus_->elementAt(irow, 0));
      
      std::stringstream sdif;sdif<<(*jt)["id"].asUInt();
      TB_DIFStatus_->elementAt(irow, 1)->addWidget(new Wt::WText(sdif.str()));
      
      std::stringstream sstatus;sstatus<<std::hex<<(*jt)["slc"].asUInt();
      TB_DIFStatus_->elementAt(irow, 2)->addWidget(new Wt::WText(sstatus.str()));
      
      std::stringstream sbcid;sbcid<<(*jt)["bcid"].asUInt64();
      TB_DIFStatus_->elementAt(irow, 3)->addWidget(new Wt::WText(sbcid.str()));
      
      std::stringstream sgtc;sgtc<<(*jt)["gtc"].asUInt();
      TB_DIFStatus_->elementAt(irow, 4)->addWidget(new Wt::WText(sgtc.str()));
      std::stringstream sbytes;sbytes<<(*jt)["bytes"].asUInt64();
      TB_DIFStatus_->elementAt(irow, 5)->addWidget(new Wt::WText(sbytes.str()));
      
      irow++;
    }
  TB_DIFStatus_->addStyleClass("table-striped",true); 
  TB_DIFStatus_->addStyleClass("table-bordered",true); 
  
  res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --trig-status -v");
  //std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  //std::cout<<jsta;
  parsingSuccessful = reader.parse(jsta["triggerStatusResponse"]["triggerStatusResult"][0].asString(),jdev1);
  
  const Json::Value& jdevs1=jdev1;

  //std::cout<<jdevs<<std::endl;
  //for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
  //  {	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
  //   std::cout<<(*jt);
  //  }
  // Header
  TB_TrigStatus_->clear();
  TB_TrigStatus_->setHeaderCount(1);
  printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
  TB_TrigStatus_->setWidth("80%");

  TB_TrigStatus_->elementAt(0, 0)->addWidget(new Wt::WText("Spill"));
  TB_TrigStatus_->elementAt(0, 1)->addWidget(new Wt::WText("busy1"));
  TB_TrigStatus_->elementAt(0, 2)->addWidget(new Wt::WText("busy2"));
  TB_TrigStatus_->elementAt(0, 3)->addWidget(new Wt::WText("busy3"));
  TB_TrigStatus_->elementAt(0, 4)->addWidget(new Wt::WText("Ecal"));
  TB_TrigStatus_->elementAt(0, 5)->addWidget(new Wt::WText("Mask"));
  irow=1;

  std::stringstream spill;spill<<" "<<jdev1["spill"].asUInt()<<" ";
  TB_TrigStatus_->elementAt(irow, 1)->addWidget(new Wt::WText(spill.str()));
  
      
  std::stringstream sdif;sdif<<jdev1["busy1"].asUInt();
  TB_TrigStatus_->elementAt(irow, 1)->addWidget(new Wt::WText(sdif.str()));
      
  std::stringstream sstatus;sstatus<<std::hex<<jdev1["busy2"].asUInt();
  TB_TrigStatus_->elementAt(irow, 2)->addWidget(new Wt::WText(sstatus.str()));
      
  std::stringstream sbcid;sbcid<<jdev1["busy3"].asUInt();
  TB_TrigStatus_->elementAt(irow, 3)->addWidget(new Wt::WText(sbcid.str()));
      
  std::stringstream sgtc;sgtc<<jdev1["ecalmask"].asUInt();
  TB_TrigStatus_->elementAt(irow, 4)->addWidget(new Wt::WText(sgtc.str()));
  std::stringstream sbytes;sbytes<<jdev1["mask"].asUInt();
  TB_TrigStatus_->elementAt(irow, 5)->addWidget(new Wt::WText(sbytes.str()));
      
    
  //TB_TrigStatus_->addStyleClass("table form-inline"); 
 //TB_TrigStatus_->addStyleClass("table-striped",true); 
}
Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  /*
   * You could read information from the environment to decide whether
   * the user has permission to start a new application
   */
  return new levbc(env);
}

int main(int argc, char **argv)
{
  /*
   * Your main method may set up some shared resources, but should then
   * start the server application (FastCGI or httpd) that starts listening
   * for requests, and handles all of the application life cycles.
   *
   * The last argument to WmRun specifies the function that will instantiate
   * new application objects. That function is executed when a new user surfs
   * to the Wt application, and after the library has negotiated browser
   * support. The function should return a newly instantiated application
   * object.
   */
  return Wt::WRun(argc, argv, &createApplication);
}

