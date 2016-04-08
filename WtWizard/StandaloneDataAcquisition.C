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

//using namespace Wt;

/*
 * A simple hello world application class which demonstrates how to react
 * to events, read input, and give feed-back.
 */
class StandaloneDataAcquisition : public Wt::WApplication
{
public:
  StandaloneDataAcquisition(const Wt::WEnvironment& env);

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
  Wt::WLineEdit *LE_RunStatus_;
  Wt::WContainerWidget *container_;
  Wt::WTabWidget *tabW_; 
  void saveData(std::string fn =".daq/last.txt");
  void readData(std::string fn =".daq/last.txt");
  void saveDataHandler();
  void readDataHandler();
  void buildParametersForm(Wt::WGroupBox *wb);
  void SaveParameters();
  void LoadParameters();

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

void StandaloneDataAcquisition::readData(std::string fn)
{
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  std::ifstream test(fn.c_str(), std::ifstream::binary);
  bool parsingSuccessful = reader.parse( test, root, false );
  if ( !parsingSuccessful )
    {
        // report to the user the failure and their locations in the document.
        std::cout  << reader.getFormattedErrorMessages()
               << "\n";
    }

    std::string CCCHost = root.get("CCCHost", "UTF-8" ).asString();
    LE_CCCHost_->setText(CCCHost);
    std::string DIFHosts = root.get("DIFHosts", "UTF-8" ).asString();
    LE_DIFHosts_->setText(DIFHosts);
    std::string DBState = root.get("DBState", "UTF-8" ).asString();
    LE_DBState_->setText(DBState);
    std::string DataDirectory = root.get("DataDirectory", "UTF-8" ).asString();
    LE_DataDirectory_->setText(DataDirectory);
    std::string ControlRegister = root.get("ControlRegister", "UTF-8" ).asString();
    LE_ControlRegister_->setText(ControlRegister);
    
    uint32_t NumberOfDIFs=root.get("NumberOfDIFs", "UTF-8" ).asUInt();
    SB_NumberOfDIFs_->setValue(NumberOfDIFs);
    uint32_t CCCPort=root.get("CCCPort", "UTF-8" ).asUInt();
    SB_CCCPort_->setValue(CCCPort);
    uint32_t DIFPort=root.get("DIFPort", "UTF-8" ).asUInt();
    SB_DIFPort_->setValue(DIFPort);
    uint32_t SaveData=root.get("SaveData", "UTF-8" ).asUInt();
    CB_SaveData_->setChecked(SaveData);

    std::cout << NumberOfDIFs << "\n";
}
void StandaloneDataAcquisition::saveData(std::string fn)
{
  std::cout<<"hello there "<<std::endl;
  std::ofstream file;// can be merged to std::ofstream file("file.txt");
  file.open(fn.c_str());
  Json::Value event;   
  event["CCCHost"]=LE_CCCHost_->text().toUTF8();
  event["CCCPort"]=SB_CCCPort_->value();
  event["DIFHosts"]=LE_DIFHosts_->text().toUTF8();
  event["DIFPort"]=SB_DIFPort_->value();
  event["DBState"]=LE_DBState_->text().toUTF8();
  event["NumberOfDIFs"]=SB_NumberOfDIFs_->value();
  event["DataDirectory"]=LE_DataDirectory_->text().toUTF8();
  event["SaveData"]=CB_SaveData_->checkState();
  event["ControlRegister"]=LE_ControlRegister_->text().toUTF8();

  file <<event;
  file.close();// is not necessary because the destructor closes the open file by default
  //fprintf(pFile,"%s\n",LE_CCCHost_->text().toUTF8().c_str());


}
void StandaloneDataAcquisition::buildParametersForm(Wt::WGroupBox *wb)
{

  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(2,2,2,2);
  wb->setLayout(vbox);

  Wt::WHBoxLayout *hbCCC = new Wt::WHBoxLayout();

  vbox->addLayout(hbCCC);


  hbCCC->addWidget(new Wt::WText("CCC Host: "));  // show some text
  LE_CCCHost_ = new Wt::WLineEdit(wb);                     // allow text input
  LE_CCCHost_->setFocus(); 
  LE_CCCHost_->changed().connect( boost::bind(&StandaloneDataAcquisition::saveData, this, ".daq/last.txt"));

  hbCCC->addWidget(LE_CCCHost_);
 

  hbCCC->addWidget(new Wt::WText("CCC PORT: "));  // show some text  
  SB_CCCPort_= new Wt::WSpinBox(wb);
  SB_CCCPort_->setValue(5000);
  SB_CCCPort_->setRange(1000,10000);
  SB_CCCPort_->setSingleStep(100);
  hbCCC->addWidget(SB_CCCPort_);
  SB_CCCPort_->changed().connect(boost::bind(&StandaloneDataAcquisition::saveData, this, ".daq/last.txt"));
  Wt::WHBoxLayout *hbDIF = new Wt::WHBoxLayout();
  vbox->addLayout(hbDIF);

  //vbox->addWidget(new WText("\n \n \n \n \n \n "));
  hbDIF->addWidget(new Wt::WText("DIF Hosts separated by comma: "));  // show some text
  LE_DIFHosts_ = new Wt::WLineEdit(wb);                     // allow text input
  hbDIF->addWidget(LE_DIFHosts_);
  LE_DIFHosts_->changed().connect(boost::bind(&StandaloneDataAcquisition::saveData, this, ".daq/last.txt"));
  hbDIF->addWidget(new Wt::WText("DIF PORT: "));  // show some text  
  SB_DIFPort_= new Wt::WSpinBox(wb);
  SB_DIFPort_->setValue(4000);
  SB_DIFPort_->setRange(1000,10000);
  SB_DIFPort_->setSingleStep(100);
  hbDIF->addWidget( SB_DIFPort_);
  SB_DIFPort_->changed().connect(boost::bind(&StandaloneDataAcquisition::saveData, this, ".daq/last.txt"));
  //vbox->addWidget(new WText("\n \n \n \n \n \n "));
  vbox->addWidget(new Wt::WText("Database State: "));  // show some text

  LE_DBState_ = new Wt::WLineEdit(wb);
  vbox->addWidget( LE_DBState_);                     // allow text input
  LE_DBState_->changed().connect(boost::bind(&StandaloneDataAcquisition::saveData, this, ".daq/last.txt"));
  vbox->addWidget(new Wt::WText("Control Register: "));  // show some text

  LE_ControlRegister_ = new Wt::WLineEdit(wb);                     // allow text input
  LE_ControlRegister_->setText("0x80000000");
  
  vbox->addWidget( LE_ControlRegister_);
  LE_ControlRegister_->changed().connect(boost::bind(&StandaloneDataAcquisition::saveData, this, ".daq/last.txt"));
  //vbox->addWidget(new WText("\n \n \n \n \n \n "));
  Wt::WHBoxLayout *hbSTO = new Wt::WHBoxLayout();
  vbox->addLayout(hbSTO);
  //vbox->addWidget(new WText("\n \n \n \n \n \n "));
  hbSTO->addWidget(new Wt::WText("Storage directory "));  // show some text
  LE_DataDirectory_ = new Wt::WLineEdit(wb);                     // allow text input
  hbSTO->addWidget(LE_DataDirectory_);
  LE_DataDirectory_->changed().connect(boost::bind(&StandaloneDataAcquisition::saveData, this, ".daq/last.txt"));
  hbSTO->addWidget(new Wt::WText("Number of DIFs: "));  // show some text  
  SB_NumberOfDIFs_= new Wt::WSpinBox(wb);
  SB_NumberOfDIFs_->setValue(1);
  SB_NumberOfDIFs_->setRange(1,255);
  SB_NumberOfDIFs_->setSingleStep(1);
  hbSTO->addWidget( SB_NumberOfDIFs_);
  SB_NumberOfDIFs_->changed().connect(boost::bind(&StandaloneDataAcquisition::saveData, this, ".daq/last.txt"));
  CB_SaveData_= new Wt::WCheckBox(wb);
  hbSTO->addWidget( CB_SaveData_);
  CB_SaveData_->changed().connect(boost::bind(&StandaloneDataAcquisition::saveData, this, ".daq/last.txt"));
  //vbox->addWidget(new WText("\n \n \n \n \n \n "));
  this->readData();
  
  Wt::WHBoxLayout *hbsave = new Wt::WHBoxLayout();
  vbox->addLayout(hbsave);
  //vbox->addWidget(new WText("\n \n \n \n \n \n "));
  hbsave->addWidget(new Wt::WText("Session Name "));  // show some text
   LE_Session_ = new Wt::WLineEdit(wb);                     // allow text input
  hbsave->addWidget(LE_Session_);
  Wt::WPushButton *saveb = new Wt::WPushButton("Save", wb); // create a button
  saveb->setMargin(5, Wt::Left);                                 // add 5 pixels margin
  Wt::WPushButton *loadb = new Wt::WPushButton("load", wb); // create a button
  loadb->setMargin(5, Wt::Left);     
                            // add 5 pixels margin
  hbsave->addWidget(saveb);
  hbsave->addWidget(loadb);
 

  saveb->clicked().connect(this,&StandaloneDataAcquisition::saveDataHandler);
  loadb->clicked().connect(this,&StandaloneDataAcquisition::readDataHandler);

}

void StandaloneDataAcquisition::saveDataHandler()
{
 std::string fn=".daq/"+LE_Session_->text().toUTF8()+".txt";
  std::cout<<fn<<" to be used"<<std::endl;
 this->saveData(fn);
}
void StandaloneDataAcquisition::readDataHandler()
{
 std::string fn=".daq/"+LE_Session_->text().toUTF8()+".txt";
  std::cout<<fn<<" to be used"<<std::endl;
 this->readData(fn);
}
/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.

DBSTATE AllInOne

Wt::WContainerWidget *container = new Wt::WContainerWidget();


*/
StandaloneDataAcquisition::StandaloneDataAcquisition(const Wt::WEnvironment& env)
  : Wt::WApplication(env),theClient_(NULL),LE_RunStatus_(NULL),TB_DIFStatus_(NULL)
{
  setTitle("Standalone Data Acquisition");                               // application title
  container_ = new Wt::WContainerWidget(root());

  tabW_ = new Wt::WTabWidget(container_);
  Wt::WGroupBox *groupBoxParam = new Wt::WGroupBox("Parametres");
  tabW_->addTab(groupBoxParam,"Parametres", Wt::WTabWidget::PreLoading);

groupBoxParam->addStyleClass("centered-example");
 buildParametersForm(groupBoxParam);

  Wt::WGroupBox *groupBoxDaq = new Wt::WGroupBox("Daq Control");
  tabW_->addTab(groupBoxDaq,"Daq Control", Wt::WTabWidget::PreLoading);

  groupBoxDaq->addStyleClass("centered-example");

  buildDaqForm(groupBoxDaq);
  Wt::WGroupBox *groupBoxDaqStatus = new Wt::WGroupBox("Daq Status");
  tabW_->addTab(groupBoxDaqStatus,"Daq Status", Wt::WTabWidget::PreLoading);

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

  b1->clicked().connect(this, &StandaloneDataAcquisition::Quit);
  tabW_->currentChanged().connect(this,&StandaloneDataAcquisition::updateMonitoringForm);

}

void StandaloneDataAcquisition::Quit()
{
  /*
   * Update the text, using text input into the nameEdit_ field.
   */
  std::string sh="http://"+this->environment().hostName()+"/test.html";		
  this->redirect(sh);

  this->quit();
}
void StandaloneDataAcquisition::buildDaqForm(Wt::WGroupBox *wb)
{

  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(2,2,2,2);
  wb->setLayout(vbox);

  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();

  vbox->addLayout(hbFSM);

  PB_Initialise_= new Wt::WPushButton("Initialise", wb);
  PB_Configure_= new Wt::WPushButton("Configure", wb);PB_Configure_->disable();
  PB_Start_= new Wt::WPushButton("Start", wb);PB_Start_->disable();
  PB_Stop_= new Wt::WPushButton("Stop", wb);PB_Stop_->disable();
  PB_Halt_= new Wt::WPushButton("Halt", wb);PB_Halt_->disable();
  PB_Destroy_= new Wt::WPushButton("Destroy", wb);PB_Destroy_->disable();
  hbFSM->addWidget(PB_Initialise_);  // show some text
  hbFSM->addWidget(PB_Configure_);  // show some text
  hbFSM->addWidget(PB_Start_);  // show some text
  hbFSM->addWidget(PB_Stop_);  // show some text
  hbFSM->addWidget(PB_Halt_);  // show some text
  hbFSM->addWidget(PB_Destroy_);  // show some text
  LE_State_ = new Wt::WLineEdit(wb);                     // allow text input
  LE_State_->setText("Destroyed");
  LE_State_->disable();
  vbox->addWidget(LE_State_);
  PB_Initialise_->clicked().connect(this,&StandaloneDataAcquisition::InitialiseButtonHandler);

  PB_Start_->clicked().connect(this,&StandaloneDataAcquisition::StartButtonHandler);
  PB_Configure_->clicked().connect(this,&StandaloneDataAcquisition::ConfigureButtonHandler);
  PB_Stop_->clicked().connect(this,&StandaloneDataAcquisition::StopButtonHandler);
  PB_Halt_->clicked().connect(this,&StandaloneDataAcquisition::HaltButtonHandler);
  PB_Destroy_->clicked().connect(this,&StandaloneDataAcquisition::DestroyButtonHandler);


}

void StandaloneDataAcquisition::InitialiseButtonHandler()
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
void StandaloneDataAcquisition::InitialiseAction()
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
void StandaloneDataAcquisition::ConfigureButtonHandler()
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
void StandaloneDataAcquisition::ConfigureAction()
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
void StandaloneDataAcquisition::StartButtonHandler()
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
void StandaloneDataAcquisition::StartAction()
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
void StandaloneDataAcquisition::StopButtonHandler()
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
void StandaloneDataAcquisition::StopAction()
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

void StandaloneDataAcquisition::HaltButtonHandler()
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
void StandaloneDataAcquisition::HaltAction()
{
  this->StopAction();
}

void StandaloneDataAcquisition::DestroyButtonHandler()
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

void StandaloneDataAcquisition::DestroyAction()
{
   theClient_->Destroy();
   printf("2\n");
   delete theClient_;
   theClient_=NULL;
}

void StandaloneDataAcquisition::createMonitoringForm(Wt::WGroupBox *wb)
{
      if (LE_RunStatus_==NULL)
	{
	  printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
	     
	  LE_RunStatus_ = new Wt::WLineEdit(wb);                     // allow text input
	  LE_RunStatus_->disable();
	  printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
	}
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
	}
      PB_RefreshStatus_= new Wt::WPushButton("Refresh", wb);
      PB_RefreshStatus_->clicked().connect(this, &StandaloneDataAcquisition::updateMonitoringForm);
      printf("%s %d\n",__PRETTY_FUNCTION__,__LINE__);
}
void StandaloneDataAcquisition::buildMonitoringForm(Wt::WGroupBox *wb)
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

void StandaloneDataAcquisition::updateMonitoringForm()
{
  if (theClient_==NULL) return;
  
  std::stringstream s;
  s<<theRunNumber_;
  LE_RunStatus_->setText(s.str());
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
  for (uint32_t i=0;i<theClient_->getNumberOfDIFs();i++)
    {
      DIFClient* d= theClient_->getDIFClient(i);
      std::map<uint32_t,DIFInfo> dmap=d->getDIFMapStatus();
      for (std::map<uint32_t,DIFInfo>::iterator itm=dmap.begin();itm!=dmap.end();itm++)
	{
	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
	  new Wt::WText(Wt::WString::fromUTF8(d->getHost()),TB_DIFStatus_->elementAt(irow, 0));

	  std::stringstream sdif;sdif<<itm->first;
	  TB_DIFStatus_->elementAt(irow, 1)->addWidget(new Wt::WText(sdif.str()));
	  std::stringstream sstatus;sstatus<<std::hex<<itm->second.status;
	  TB_DIFStatus_->elementAt(irow, 2)->addWidget(new Wt::WText(sstatus.str()));
	  std::stringstream sbcid;sbcid<<itm->second.lastReceivedBCID;
	  TB_DIFStatus_->elementAt(irow, 3)->addWidget(new Wt::WText(sbcid.str()));
	  std::stringstream sgtc;sgtc<<itm->second.lastReceivedGTC;
	  TB_DIFStatus_->elementAt(irow, 4)->addWidget(new Wt::WText(sgtc.str()));
	  std::stringstream sbytes;sbytes<<itm->second.bytesReceived;
	  TB_DIFStatus_->elementAt(irow, 5)->addWidget(new Wt::WText(sbytes.str()));
	  irow++;
	}
    }
  // TB_DIFStatus_->addStyleClass("table form-inline"); 
 TB_DIFStatus_->addStyleClass("table-striped",true); 
 TB_DIFStatus_->addStyleClass("table-bordered",true); 
}
Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  /*
   * You could read information from the environment to decide whether
   * the user has permission to start a new application
   */
  return new StandaloneDataAcquisition(env);
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

