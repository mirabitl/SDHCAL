/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <stdio.h>

#include <iostream>
#include <sstream>
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
#include <Wt/WPanel>
#include <Wt/WText>
#include <Wt/WTable>
#include <Wt/WDialog>
#include <Wt/WMessageBox>
#include <Wt/WLabel>

// c++0x only, for std::bind
// #include <functional>
using namespace  std;
#include "LCIOWritterInterface.h"
#include "DIFMultiClient.h"
#include "OracleDIFDBManager.h"
#include "Tokenizer.h"
#include <json/json.h>
//using namespace Wt;
 #define STEP printf("%s %d\n",__FUNCTION__,__LINE__)
/*
 * A simple hello world application class which demonstrates how to react
 * to events, read input, and give feed-back.
 */
class wlevbc : public Wt::WApplication
{
public:
  wlevbc(const Wt::WEnvironment& env);

private:
  uint32_t theRunNumber_;

  Wt::WLineEdit *LE_CCCHost_;
  Wt::WLineEdit *LE_DIFHosts_;
  Wt::WLineEdit *LE_DataDirectory_;
  Wt::WLineEdit *LE_DBState_;
  Wt::WLineEdit *LE_ControlRegister_;
  Wt::WLineEdit *LE_Session_;
  
  Wt::WSpinBox  *SB_CCCPort_;
  Wt::WSpinBox  *SB_DIFPort_;
  Wt::WSpinBox  *SB_NumberOfDIFs_;
  Wt::WSpinBox  *SB_DefaultRunNumber_;
  Wt::WLineEdit *LE_State_;
  Wt::WTable *TB_DIFStatus_;
  Wt::WTable *TB_TrigStatus_;
  Wt::WLineEdit *LE_RunStatus_;
  Wt::WLineEdit *LE_RunNumber_;
  Wt::WLineEdit *LE_EventNumber_;

  // Used widgets
  Wt::WPushButton *PB_Service_;
  Wt::WPushButton *PB_LV_;
  Wt::WPushButton *PB_Initialise_;
  Wt::WPushButton *PB_Configure_;
  Wt::WPushButton *PB_Start_;
  Wt::WPushButton *PB_Stop_;
  Wt::WPushButton *PB_Destroy_;
  Wt::WPushButton *PB_Status_;
  Wt::WCheckBox *CB_DIF_;

  Wt::WPushButton *PB_TReset_;
  Wt::WPushButton *PB_TResume_;
  Wt::WPushButton *PB_TPause_;

  Wt::WSpinBox  *SB_FChan_;
  Wt::WSpinBox  *SB_LChan_;
  Wt::WSpinBox  *SB_Vol_;
  Wt::WPushButton *PB_HVCreate_;
  Wt::WPushButton *PB_HVStatus_;
  Wt::WPushButton *PB_HVSet_;
  Wt::WPushButton *PB_HVOn_;
  Wt::WPushButton *PB_HVOff_;
  Wt::WPushButton *PB_JCStatus_;
  Wt::WPushButton *PB_JCStart_;
  Wt::WPushButton *PB_JCKill_;
  
  Wt::WContainerWidget *container_;
  Wt::WTabWidget *tabW_; 
  Wt::WText* tDaqStatus_;
  Wt::WText* tHVStatus_;
  Wt::WText* tJCStatus_;  
  std::string currentState_;
  Json::Value jLV_;
  Json::Value jJC_,jAvail_;
  uint32_t currentRun_,currentEvent_,currentElog_;
  std::string currentSessionId_;
  void Quit();
  void buildDaqForm(Wt::WPanel *wb);
  void buildHVForm(Wt::WPanel *wb);
  void buildJCForm(Wt::WPanel *wb);
  void fillDaqStatus(Wt::WPanel *wb);
  void fillHVStatus(Wt::WPanel *wb);
  void fillHVControl(Wt::WPanel *wb);
  void fillJCStatus(Wt::WPanel *wb);
  void fillJCControl(Wt::WPanel *wb);
  void ServiceButtonHandler();
  void LVButtonHandler();
  void InitialiseButtonHandler();
  void ConfigureButtonHandler();
  void StartButtonHandler();
  void StopButtonHandler();
  void DestroyButtonHandler();
  void TResetButtonHandler();
  void TResumeButtonHandler();
  void TPauseButtonHandler();
  void HVSetButtonHandler();
  void HVOnButtonHandler();
  void HVOffButtonHandler();

  void updateMonitoringForm();

  void toggleButtons();
  std::string daqStatus();
  std::string hvStatus();
  std::string jobStatus();

  void checkState();
  void checkSlowControl();
  void checkLV();
  void checkJobControl();
  bool isLVOn() {return jLV_["vout"].asFloat()>1.;}
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

std::string lcexec(std::string s)
{
  std::string daqpath=getenv("DAQDIR");
  std::stringstream os;
  os<<"cd "<<daqpath<<"/levbdim_daq;. ./levbdimrc;./webc.py "<<s;
  std::cout<<"executing =====>"<<os.str()<<std::endl;
  return exec(os.str().c_str());
}


wlevbc::wlevbc(const Wt::WEnvironment& env)
  : Wt::WApplication(env),LE_RunStatus_(NULL),TB_DIFStatus_(NULL),LE_RunNumber_(NULL),LE_EventNumber_(NULL),currentElog_(0),currentSessionId_("")
{

  Wt::WApplication::instance()->useStyleSheet("bootstrap/css/bootstrap.min.css");
  std::string res=lcexec(" --webstatus -v");
  Json::Reader reader;
  Json::Value jsta,jdev1;
  //std::cout<<res<<std::endl;
  jAvail_.clear();
  bool parsingSuccessful = reader.parse(res,jAvail_);
  if (jAvail_["JOB"].asString().compare("DEAD")==0 ||
      jAvail_["SLOW"].asString().compare("DEAD")==0 ||
      jAvail_["DAQ"].asString().compare("DEAD")==0)
    {
      Wt::WMessageBox *messageBox = new Wt::WMessageBox
	("Error",
	 "<p>Missing web services</p>"
	 "<p>Please start it and reload the page </p>",
	 Wt::Information, Wt::Yes);

    messageBox->setModal(false);

    messageBox->buttonClicked().connect(std::bind([=] () {
	  //if (messageBox->buttonResult() == Wt::Yes)
	  // out->setText("Reload the page after ");
	delete messageBox;
    }));

    messageBox->show();
    return;
    }
  std::cout<<"Youpi Ya "<<res<<std::endl;
  //  std::cout<<jsta;
  res=lcexec(" --jc-cre");

  if (jAvail_["JOB"].asString().compare("RUNNING")==0)
    {
      res=lcexec(" --slc-cre");
      res=lcexec(" --daq-cre");
    }
  setTitle("GRPC forever");                               // application title
  container_ = new Wt::WContainerWidget(root());
  container_->setStyleClass("bootstrap");
  tabW_ = new Wt::WTabWidget(container_);
 

  Wt::WPanel *groupBoxJC = new Wt::WPanel();
  groupBoxJC->setTitle("Job Control");
  groupBoxJC->setCollapsible(true);
  
  tabW_->addTab(groupBoxJC,"Job Control");//, Wt::WTabWidget::PreLoading);

  groupBoxJC->addStyleClass("centered-example");
  buildJCForm(groupBoxJC);




  Wt::WPanel *groupBoxHV = new Wt::WPanel();
  groupBoxHV->setTitle("HV Control");
  groupBoxHV->setCollapsible(true);
  
  tabW_->addTab(groupBoxHV,"HV Control");//, Wt::WTabWidget::PreLoading);

  groupBoxHV->addStyleClass("centered-example");
  buildHVForm(groupBoxHV);

  
  //Wt::WGroupBox *groupBoxDaq = new Wt::WGroupBox("Daq Control");
  Wt::WPanel *groupBoxDaq = new Wt::WPanel();
  groupBoxDaq->setTitle("Daq Control");
  groupBoxDaq->setCollapsible(false);
  
  tabW_->addTab(groupBoxDaq,"Daq Control");//, Wt::WTabWidget::PreLoading);

  groupBoxDaq->addStyleClass("centered-example");
  buildDaqForm(groupBoxDaq);


    //Wt::WGroupBox *groupBoxDaq = new Wt::WGroupBox("Daq Control");w
  
  //tabW_->addTab(groupBoxDaq,"Daq Control");//, Wt::WTabWidget::PreLoading);

  //groupBoxDaq->addStyleClass("centered-example");

  // Wt::WGroupBox *groupBoxDaqStatus = new Wt::WGroupBox("Daq Status");
  // tabW_->addTab(groupBoxDaqStatus,"Daq Status");//, Wt::WTabWidget::PreLoading);

  // groupBoxDaqStatus->addStyleClass("centered-example");

  // buildMonitoringForm(groupBoxDaqStatus);

    tabW_->setStyleClass("tabwidget");

   root()->addWidget(new Wt::WBreak());                       // insert a line break
   Wt::WPushButton *b1 = new Wt::WPushButton("Kill the Session", root()); // create a button
   b1->setMargin(5, Wt::Left);                                 // add 5 pixels margin

   root()->addWidget(new Wt::WBreak());                       // insert a line break



  // /*
  //  * Connect signals with slots
  //  *
  //  * - simple Wt-way
  //  */

  b1->clicked().connect(this, &wlevbc::Quit);
  tabW_->currentChanged().connect(this,&wlevbc::updateMonitoringForm);

}

void wlevbc::Quit()
{
  /*
   * Update the text, using text input into the nameEdit_ field.
   */
  std::string sh="http://"+this->environment().hostName()+"/test.html";		
  this->redirect(sh);

  this->quit();
}


std::string wlevbc::jobStatus()
{
  std::stringstream os;
  std::string res;

  //if (theClient_==NULL) return;
  Json::Reader reader;
  Json::Value jsta,jdev1;
  

  res=lcexec(" --jc-status -v");
  //std::cout<<res<<std::endl;
  jsta.clear();
  bool parsingSuccessful = reader.parse(res,jsta);
  //  std::cout<<jsta;
  jdev1.clear();
  jdev1=jsta["answer"]["ANSWER"];
  //std::cout<<jdev1;
  const Json::Value& jdevs= jdev1;
  //std::cout<<jdevs<<std::endl;
  //for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
  //  {	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
  //   std::cout<<(*jt);
  //  }
  // Header
  os<<"<h4> JOB Status </h4>"<<std::endl;
  os<<"<table style=\"width:100%\">";
  os<<"<tr><th style=\"text-align:left\">PID</th><th style=\"text-align:left\">NAME</th> <th style=\"text-align:left\">HOST</th><th style=\"text-align:left\">STATUS</th><th style=\"text-align:left\">In Daq</th></tr>";
  uint32_t irow=1;
  for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
    {	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
      //std::cout<<(*jt);

      os<<"<tr>";
      os<<"<td>"<<(*jt)["PID"].asUInt()<<"</td>";
      os<<"<td>"<<(*jt)["NAME"].asString()<<"</td>";
      os<<"<td>"<<(*jt)["HOST"].asString()<<"</td>";
      os<<"<td>"<<(*jt)["STATUS"].asString()<<"</td>";
      os<<"<td>"<<(*jt)["DAQ"].asString()<<"</td>";
      os<<"</tr>";
      irow++;
    }
  os<<"</table>"<<std::endl;
  std::cout<<os.str();
  return os.str();
}


std::string wlevbc::hvStatus()
{
  std::stringstream os;
  std::string res=lcexec(" --slc-pt -v");

  //if (theClient_==NULL) return;
  Json::Reader reader;
  Json::Value jsta;
  
  os<<"<h4> Environment </h4>"<<std::endl;
  
  jsta.clear();
  bool parsingSuccessful = reader.parse(res,jsta);
  //std::cout<<jsta<<std::endl;
  Json::Value jdev1=jsta["answer"]["ANSWER"];
  //parsingSuccessful = reader.parse(jsta["PTResponse"]["PTResult"][0].asString(),jdev1);
  //std::cout<<jdev1;
  os<<"<table style=\"width:100%\">";
  os<<"<tr><th style=\"text-align:left\">Pression</th><th style=\"text-align:left\">Temperature</th> </tr>";

  os<<"<tr><td>"<<jdev1["P"].asFloat()<<"</td>";
  os<<"<td>"<<jdev1["T"].asFloat()<<"</td></tr>";
  os<<"</table>";


  res=lcexec(" --slc-hvstatus -v");
  //std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  //  std::cout<<jsta;
  jdev1=jsta["answer"]["ANSWER"];
  //std::cout<<jdev1;
  const Json::Value& jdevs= jdev1;
  //std::cout<<jdevs<<std::endl;
  //for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
  //  {	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
  //   std::cout<<(*jt);
  //  }
  // Header
  os<<"<h4> HV Status </h4>"<<std::endl;
  os<<"<table style=\"width:100%\">";
  os<<"<tr><th style=\"text-align:left\">Channel</th><th style=\"text-align:left\">VSET</th> <th style=\"text-align:left\">ISET</th><th style=\"text-align:left\">VOUT</th><th style=\"text-align:left\">IOUT</th></tr>";
  uint32_t irow=1;
  for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
    {	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
      //std::cout<<(*jt);

      os<<"<tr>";
      os<<"<td>"<<(*jt)["channel"].asUInt()<<"</td>";
      os<<"<td>"<<(*jt)["vset"].asFloat()<<"</td>";
      os<<"<td>"<<(*jt)["iset"].asFloat()<<"</td>";
      os<<"<td>"<<(*jt)["vout"].asFloat()<<"</td>";
      os<<"<td>"<<(*jt)["iout"].asFloat()<<"</td>";
      os<<"</tr>";
      irow++;
    }
  os<<"</table>"<<std::endl;
  std::cout<<os.str();
  return os.str();
}


std::string wlevbc::daqStatus()
{
  std::stringstream os;
  std::string res=lcexec(" --daq-state -v");

  //if (theClient_==NULL) return;
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(res,jsta);
  std::cout<<res<<std::endl;
  std::cout<<jsta<<std::endl;
  std::cout<<jsta["STATE"].asString()<<std::endl;
  currentState_=jsta["STATE"].asString();
  os<<"<h4> State :"<<jsta["STATE"].asString()<<"</h4>";
  //Event builder
  os<<"<h4> Low Voltage </h4>"<<std::endl;
  /*res=lcexec(" --daq-lvsta -v");
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  Json::Value jdev1;
  std::cout<<"JSTA"<<jsta;
  parsingSuccessful = reader.parse(jsta["LVStatusResponse"]["LVStatusResult"][0].asString(),jdev1);
  std::cout<<jdev1<<jdev1.empty()<<jsta["LVStatusResponse"]["LVStatusResult"][0];
  STEP;*/
  if (!jLV_.empty())
    {
      os<<"<table style=\"width:100%\">";
      os<<"<tr><th style=\"text-align:left\">VSET</th><th style=\"text-align:left\">VOUT</th> <th style=\"text-align:left\">IOUT</th></tr>";
      
      os<<"<tr><td>"<<jLV_["vset"].asFloat()<<"</td>";
      os<<"<td>"<<jLV_["vout"].asFloat()<<"</td>";
      os<<"<td>"<<jLV_["iout"].asFloat()<<"</td></tr>";
      os<<"</table>";
    }

  res=lcexec(" --daq-evb -v");
  std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  Json::Value jdev1;
  jdev1.clear();
  //parsingSuccessful = reader.parse(jsta["shmStatusResponse"]["shmStatusResult"][0].asString(),jdev1);
  jdev1=jsta["answer"];
  //std::cout<<currentState_<<currentState_.compare("INITIALISED")<<std::endl;
  //std::cout<<jdev1<<std::endl;
  if (parsingSuccessful && !jdev1.empty() && jsta["status"].asString().compare("OK")==0)
    {
      try {
	currentRun_=jdev1["run"].asUInt();
	currentEvent_=jdev1["event"].asUInt();
      os<<"<h4> Event Builder:</h4>";
      os<<"<p><b>Run:</b> "<<jdev1["run"].asString()<<"</p>";
      os<<"<p><b>Event:</b> "<<jdev1["event"].asString()<<"</p>";
      }
      catch(...)
	{
	}
    }

  os<<"<h4> Trigger Status MDCC:</h4>"<<std::endl;
  res=lcexec(" --trig-status -v");
  //std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  //std::cout<<jsta;
  //parsingSuccessful = reader.parse(jsta["triggerStatusResponse"]["triggerStatusResult"][0].asString(),jdev1);
  jdev1=jsta["answer"]["COUNTERS"];
  if (parsingSuccessful && !jdev1["spill"].empty() && jsta["status"].asString().compare("OK")==0)
    {

      os<<"<table style=\"width:100%\">";
      os<<"<tr><th style=\"text-align:left\">Spill</th><th style=\"text-align:left\">SDHCAL</th> <th style=\"text-align:left\">ECAL</th><th style=\"text-align:left\" >Spare</th><th style=\"text-align:left\">ECAL mask</th><th style=\"text-align:left\">Mask</th></tr>";
      
      os<<"<tr><td>"<<jdev1["spill"].asUInt()<<"</td>";
      os<<"<td>"<<jdev1["busy1"].asUInt()<<"</td>";
      os<<"<td>"<<jdev1["busy2"].asUInt()<<"</td>";
      os<<"<td>"<<jdev1["busy3"].asUInt()<<"</td>";
      os<<"<td>"<<jdev1["ecalmask"].asUInt()<<"</td>";
      os<<"<td>"<<jdev1["mask"].asUInt()<<"</td></tr>";
      os<<"</table>";
    }
  if (!CB_DIF_->isChecked()) return os.str();
  res=lcexec(" --daq-status -v");
  //std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  //parsingSuccessful = reader.parse(jsta["statusResponse"]["statusResult"][0].asString(),jdev1);
  jdev1=jsta["answer"]["diflist"];
  const Json::Value& jdevs= jdev1;
  //std::cout<<jdevs<<std::endl;
  //for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
  //  {	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
  //   std::cout<<(*jt);
  //  }
  // Header
  os<<"<h4> DIF Status </h4>"<<std::endl;
  os<<"<table style=\"width:100%\">";
  os<<"<tr><th style=\"text-align:left\">Host</th><th style=\"text-align:left\">DIF</th> <th style=\"text-align:left\">Status</th><th style=\"text-align:left\">BCID</th><th style=\"text-align:left\">GTC</th><th style=\"text-align:left\">Bytes</th><th style=\"text-align:left\">State</th></tr>";
  uint32_t irow=1;
  for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
    {	  //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
      //std::cout<<(*jt);

      os<<"<tr>";
      os<<"<td>"<<(*jt)["host"].asString()<<"</td>";
      os<<"<td>"<<(*jt)["id"].asUInt()<<"</td>";
      os<<"<td>"<<std::hex<<(*jt)["slc"].asUInt()<<std::dec<<"</td>";
      os<<"<td>"<<(*jt)["bcid"].asUInt64()<<"</td>";
      os<<"<td>"<<(*jt)["gtc"].asUInt()<<"</td>";
      os<<"<td>"<<(*jt)["bytes"].asUInt64()<<"</td>";
      os<<"<td>"<<(*jt)["state"].asString()<<"</td>";
      os<<"</tr>";
      irow++;
    }
  os<<"</table>"<<std::endl;
  std::cout<<os.str();
  return os.str();
}
void wlevbc::fillDaqStatus(Wt::WPanel *wp)
{
  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(12,12,12,12);
  wb->setLayout(vbox);
  //wb->resize(200,200);
  
  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();
  //  LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();
  // this->checkState();
  // std::stringstream s;
  // s<<"<h4>State:"<<currentState_<<"</h4>";
  tDaqStatus_=new Wt::WText(this->daqStatus(),Wt::XHTMLText);
  hbFSM->addWidget(tDaqStatus_);
  
  vbox->addLayout(hbFSM);
}
void wlevbc::buildDaqForm(Wt::WPanel *wp)
{
  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(12,12,12,12);
  wb->setLayout(vbox);
  //wb->resize(200,200);

  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();



  PB_Service_= new Wt::WPushButton("Service", wb);
  PB_LV_= new Wt::WPushButton("LV Switch", wb);
  PB_Initialise_= new Wt::WPushButton("Initialise", wb);
  PB_Configure_= new Wt::WPushButton("Configure", wb);//PB_Configure_->disable();
  PB_Start_= new Wt::WPushButton("Start", wb);//PB_Start_->disable();
  PB_Stop_= new Wt::WPushButton("Stop", wb);//PB_Stop_->disable();
  PB_Destroy_= new Wt::WPushButton("Destroy", wb);//PB_Destroy_->disable();
  PB_Status_= new Wt::WPushButton("Status", wb);//PB_Destroy_->disable();
  CB_DIF_ = new Wt::WCheckBox("Check DIF", wb);
  CB_DIF_->setChecked(false);
  hbFSM->addWidget(PB_Service_);  // show some text

  hbFSM->addWidget(PB_Initialise_);  // show some text
  hbFSM->addWidget(PB_Configure_);  // show some text
  hbFSM->addWidget(PB_Start_);  // show some text
  hbFSM->addWidget(PB_Stop_);  // show some text
  hbFSM->addWidget(PB_Destroy_);  // show some text
  vbox->addLayout(hbFSM);

  Wt::WHBoxLayout *hbinfo = new Wt::WHBoxLayout();
  //  Wt::WGroupBox *groupBox = new Wt::WGroupBox();
  // LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();
  hbinfo->addWidget(PB_LV_);  // show some text  
  hbinfo->addWidget(PB_Status_);  // show some text
  hbinfo->addWidget(CB_DIF_);  // show some text

  PB_TReset_= new Wt::WPushButton("Reset Trigger Counter", wb);//PB_Destroy_->disable();
  PB_TResume_= new Wt::WPushButton("Resume Trigger", wb);//PB_Destroy_->disable();
  PB_TPause_= new Wt::WPushButton("Pause Trigger", wb);//PB_Destroy_->disable();
  hbinfo->addWidget(PB_TReset_);  // show some text  
  hbinfo->addWidget(PB_TResume_);  // show some text
  hbinfo->addWidget(PB_TPause_);  // show some text
  

  vbox->addLayout(hbinfo);

  
  Wt::WPanel *pDaqStatus = new Wt::WPanel();
  pDaqStatus->setTitle("Daq Status");
  pDaqStatus->setCollapsible(true);
  fillDaqStatus(pDaqStatus);
  vbox->addWidget(pDaqStatus);
  
   this->checkState();
  PB_Service_->clicked().connect(this,&wlevbc::ServiceButtonHandler);
  PB_LV_->clicked().connect(this,&wlevbc::LVButtonHandler);
  PB_Initialise_->clicked().connect(this,&wlevbc::InitialiseButtonHandler);

  PB_Start_->clicked().connect(this,&wlevbc::StartButtonHandler);
  PB_Configure_->clicked().connect(this,&wlevbc::ConfigureButtonHandler);
  PB_Stop_->clicked().connect(this,&wlevbc::StopButtonHandler);
  PB_Destroy_->clicked().connect(this,&wlevbc::DestroyButtonHandler);
  PB_Status_->clicked().connect(this,&wlevbc::checkState);

  PB_TReset_->clicked().connect(this,&wlevbc::TResetButtonHandler);
  PB_TResume_->clicked().connect(this,&wlevbc::TResumeButtonHandler);
  PB_TPause_->clicked().connect(this,&wlevbc::TPauseButtonHandler);

}
void wlevbc::fillHVControl(Wt::WPanel *wp)
{
  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(2,2,2,2);
  wb->setLayout(vbox);
  //wb->resize(200,200);
  
  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();
  //  LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();
  // this->checkState();
  // std::stringstream s;
  // s<<"<h4>State:"<<currentState_<<"</h4>";

  if (jAvail_["SLOW"].asString().compare("CREATED")==0)
    {
      PB_HVCreate_= new Wt::WPushButton("Create", wb);//PB_Destroy_->disable();
      hbFSM->addWidget(PB_HVCreate_);
      PB_HVCreate_->clicked().connect(std::bind([=] () {
	std::string res=lcexec(" --slc-cre");
	PB_HVCreate_->disable();
    }));
      
    }
    
  PB_HVStatus_= new Wt::WPushButton("Status", wb);//PB_Destroy_->disable();
  hbFSM->addWidget(PB_HVStatus_);
  hbFSM->addWidget(new Wt::WText("First Channel: "));  // show some text  
  SB_FChan_= new Wt::WSpinBox(wb);
  SB_FChan_->setValue(0);
  SB_FChan_->setRange(0,55);
  SB_FChan_->setSingleStep(1);
  SB_FChan_->setMargin(5);
  SB_FChan_->setLineHeight(5);
  hbFSM->addWidget(SB_FChan_);
  hbFSM->addWidget(new Wt::WText("Last Channel: "));  // show some text  
  SB_LChan_= new Wt::WSpinBox(wb);
  SB_LChan_->setValue(0);
  SB_LChan_->setRange(0,55);
  SB_LChan_->setSingleStep(1);
  SB_LChan_->setMargin(5);
  SB_LChan_->setLineHeight(5);

  hbFSM->addWidget(SB_LChan_);
  hbFSM->addWidget(new Wt::WText("Voltage: "));  // show some text  
  SB_Vol_= new Wt::WSpinBox(wb);
  SB_Vol_->setValue(0);
  SB_Vol_->setRange(0,8000);
  SB_Vol_->setSingleStep(50);
  SB_Vol_->setMargin(5);
  SB_Vol_->setLineHeight(5);

  hbFSM->addWidget(SB_Vol_);

  PB_HVSet_= new Wt::WPushButton("Set", wb);//PB_Destroy_->disable();
  hbFSM->addWidget(PB_HVSet_);
  PB_HVOn_= new Wt::WPushButton("On", wb);//PB_Destroy_->disable();
  hbFSM->addWidget(PB_HVOn_);
  PB_HVOff_= new Wt::WPushButton("Off", wb);//PB_Destroy_->disable();
  hbFSM->addWidget(PB_HVOff_);

  
  vbox->addLayout(hbFSM);
  
  PB_HVStatus_->clicked().connect(this,&wlevbc::checkSlowControl);
  PB_HVSet_->clicked().connect(this,&wlevbc::HVSetButtonHandler);
  PB_HVOn_->clicked().connect(this,&wlevbc::HVOnButtonHandler);
  PB_HVOff_->clicked().connect(this,&wlevbc::HVOffButtonHandler);
}

void wlevbc::fillHVStatus(Wt::WPanel *wp)
{
  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(12,12,12,12);
  wb->setLayout(vbox);
  //wb->resize(200,200);
  
  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();
  //  LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();
  // this->checkState();
  // std::stringstream s;
  // s<<"<h4>State:"<<currentState_<<"</h4>";
  tHVStatus_=new Wt::WText(this->hvStatus(),Wt::XHTMLText);
  hbFSM->addWidget(tHVStatus_);
  
  vbox->addLayout(hbFSM);
}

void wlevbc::buildHVForm(Wt::WPanel *wp)
{
  STEP;
  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(12,12,12,12);
  wb->setLayout(vbox);

  //wb->resize(200,200);

  //Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();
  STEP;


  /*  PB_Service_= new Wt::WPushButton("Service", wb);
  PB_LV_= new Wt::WPushButton("LV Switch", wb);
  PB_Initialise_= new Wt::WPushButton("Initialise", wb);
  PB_Configure_= new Wt::WPushButton("Configure", wb);//PB_Configure_->disable();
  PB_Start_= new Wt::WPushButton("Start", wb);//PB_Start_->disable();
  PB_Stop_= new Wt::WPushButton("Stop", wb);//PB_Stop_->disable();
  PB_Destroy_= new Wt::WPushButton("Destroy", wb);//PB_Destroy_->disable();
  */
  //  PB_HVStatus_= new Wt::WPushButton("Status", wb);//PB_Destroy_->disable();
  /*
  CB_DIF_ = new Wt::WCheckBox("Check DIF", wb);
  CB_DIF_->setChecked(false);
  hbFSM->addWidget(PB_Service_);  // show some text
  hbFSM->addWidget(PB_LV_);  // show some text
  hbFSM->addWidget(PB_Initialise_);  // show some text
  hbFSM->addWidget(PB_Configure_);  // show some text
  hbFSM->addWidget(PB_Start_);  // show some text
  hbFSM->addWidget(PB_Stop_);  // show some text
  hbFSM->addWidget(PB_Destroy_);  // show some text
  */
  // hbFSM->addWidget(PB_HVStatus_);  // show some text
  // hbFSM->addWidget(CB_DIF_);  // show some text
  // vbox->addLayout(hbFSM);
  STEP;
  Wt::WPanel *pControl = new Wt::WPanel();
  pControl->setTitle("HV Control");
  pControl->setCollapsible(true);
  STEP;
  fillHVControl(pControl);
  STEP;
  vbox->addWidget(pControl);

  
  Wt::WPanel *pStatus = new Wt::WPanel();
  pStatus->setTitle("Slow Control Status");
  pStatus->setCollapsible(true);
  STEP;
  fillHVStatus(pStatus);
  STEP;
  vbox->addWidget(pStatus);
  STEP;
  // Wt::WHBoxLayout *hbinfo = new Wt::WHBoxLayout();
  // Wt::WGroupBox *groupBox = new Wt::WGroupBox();
  // LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();
  //this->checkState();
  // groupBox->addWidget(LE_State_);
  // hbinfo->addWidget(groupBox);
  // vbox->addLayout(hbinfo);
  /*
  PB_Service_->clicked().connect(this,&wlevbc::ServiceButtonHandler);
  PB_LV_->clicked().connect(this,&wlevbc::LVButtonHandler);
  PB_Initialise_->clicked().connect(this,&wlevbc::InitialiseButtonHandler);

  PB_Start_->clicked().connect(this,&wlevbc::StartButtonHandler);
  PB_Configure_->clicked().connect(this,&wlevbc::ConfigureButtonHandler);
  PB_Stop_->clicked().connect(this,&wlevbc::StopButtonHandler);
  PB_Destroy_->clicked().connect(this,&wlevbc::DestroyButtonHandler);
  */
  //PB_HVStatus_->clicked().connect(this,&wlevbc::checkSlowControl);
  STEP;

}
void wlevbc::checkSlowControl()
{
  tHVStatus_->setText(this->hvStatus());
}


///////////////////////////////////////////////////////////////////////////////////


void wlevbc::fillJCControl(Wt::WPanel *wp)
{
  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(2,2,2,2);
  wb->setLayout(vbox);
  //wb->resize(200,200);
  
  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();
  //  LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();
  // this->checkState();
  // std::stringstream s;
  // s<<"<h4>State:"<<currentState_<<"</h4>";

  PB_JCStatus_= new Wt::WPushButton("Status", wb);//PB_Destroy_->disable();
  hbFSM->addWidget(PB_JCStatus_);
  PB_JCStart_= new Wt::WPushButton("Start", wb);//PB_Destroy_->disable();
  hbFSM->addWidget(PB_JCStart_);
  PB_JCKill_= new Wt::WPushButton("Kill", wb);//PB_Destroy_->disable();
  hbFSM->addWidget(PB_JCKill_);
  
  vbox->addLayout(hbFSM);
  
  PB_JCStatus_->clicked().connect(this,&wlevbc::checkJobControl);
  PB_JCKill_->clicked().connect(std::bind([=] () {
	std::string res=lcexec(" --jc-kill ");
	::sleep(1);
        checkJobControl();
    }));
  PB_JCStart_->clicked().connect(std::bind([=] () {
	std::string res=lcexec(" --jc-start ");
	::sleep(1);
	checkJobControl();
    }));
  //  PB_JCStart_->clicked().connect(this,&wlevbc::HVSetButtonHandler);
  // PB_JCKill_->clicked().connect(this,&wlevbc::HVOnButtonHandler);

}

void wlevbc::fillJCStatus(Wt::WPanel *wp)
{
  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(12,12,12,12);
  wb->setLayout(vbox);
  //wb->resize(200,200);
  
  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();
  //  LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();
  // this->checkState();
  // std::stringstream s;
  // s<<"<h4>State:"<<currentState_<<"</h4>";
  tJCStatus_=new Wt::WText(this->jobStatus(),Wt::XHTMLText);
  hbFSM->addWidget(tJCStatus_);
  
  vbox->addLayout(hbFSM);
}

void wlevbc::buildJCForm(Wt::WPanel *wp)
{

  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(12,12,12,12);
  wb->setLayout(vbox);

  Wt::WPanel *pControl = new Wt::WPanel();
  pControl->setTitle("Job Control");
  pControl->setCollapsible(true);
  STEP;
  fillJCControl(pControl);
  vbox->addWidget(pControl);

  
  Wt::WPanel *pStatus = new Wt::WPanel();
  pStatus->setTitle("Job Status");
  pStatus->setCollapsible(true);
  STEP;
  fillJCStatus(pStatus);
  STEP;
  vbox->addWidget(pStatus);
  STEP;
  // Wt::WHBoxLayout *hbinfo = new Wt::WHBoxLayout();
  // Wt::WGroupBox *groupBox = new Wt::WGroupBox();
  // LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();
  //this->checkState();
  // groupBox->addWidget(LE_State_);
  // hbinfo->addWidget(groupBox);
  // vbox->addLayout(hbinfo);
  /*
  PB_Service_->clicked().connect(this,&wlevbc::ServiceButtonHandler);
  PB_LV_->clicked().connect(this,&wlevbc::LVButtonHandler);
  PB_Initialise_->clicked().connect(this,&wlevbc::InitialiseButtonHandler);

  PB_Start_->clicked().connect(this,&wlevbc::StartButtonHandler);
  PB_Configure_->clicked().connect(this,&wlevbc::ConfigureButtonHandler);
  PB_Stop_->clicked().connect(this,&wlevbc::StopButtonHandler);
  PB_Destroy_->clicked().connect(this,&wlevbc::DestroyButtonHandler);
  */
  //PB_HVStatus_->clicked().connect(this,&wlevbc::checkSlowControl);
  STEP;

}
void wlevbc::checkJobControl()
{
  tJCStatus_->setText(this->jobStatus());
}

//////////////////////////////////////////////////////////////////////////////////



void wlevbc::checkState()
{
  std::string res=lcexec(" --daq-state -v");
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(res,jsta);
  std::cout<<res<<std::endl;
  std::cout<<jsta<<std::endl;
  std::cout<<jsta["STATE"].asString()<<std::endl;

  //LE_State_->setText(jsta["stateResponse"]["stateResult"][0].asString());
  currentState_=jsta["STATE"].asString();

  this->checkLV();
  this->toggleButtons();
  tDaqStatus_->setText(this->daqStatus());
 
}
void wlevbc::toggleButtons()
{
  if (currentState_.compare("DISCOVERED")==0 )
    {
      PB_Service_->enable();
      PB_Initialise_->disable();
      PB_Configure_->disable();
      PB_Start_->disable();
      PB_Stop_->disable();
      PB_Destroy_->disable();
      
    }
  if (currentState_.compare("PREPARED")==0 )
    {
      PB_Service_->enable();
      PB_Initialise_->enable();
      PB_Configure_->disable();
      PB_Start_->disable();
      PB_Stop_->disable();
      PB_Destroy_->disable();
      
    }
  if (currentState_.compare("INITIALISED")==0 )
    {
      PB_Service_->disable();
      PB_Initialise_->disable();
      PB_Configure_->enable();
      PB_Start_->disable();
      PB_Stop_->disable();
      PB_Destroy_->enable();
      
    }
  if (currentState_.compare("CONFIGURED")==0 )
    {
      PB_Service_->disable();
      PB_Initialise_->disable();
      PB_Configure_->enable();
      PB_Start_->enable();
      PB_Stop_->disable();
      PB_Destroy_->enable();
      
    }
  if (currentState_.compare("RUNNING")==0 )
    {
      PB_Service_->disable();
      PB_Initialise_->disable();
      PB_Configure_->disable();
      PB_Start_->disable();
      PB_Stop_->enable();
      PB_Destroy_->disable();
      
    }
  if (isLVOn())
    PB_LV_->setStyleClass("btn-success");
  else
    PB_LV_->setStyleClass("btn-danger");
  
}



void wlevbc::ServiceButtonHandler()
{
  std::string res=lcexec(" --daq-state -v");
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(res,jsta);
  std::cout<<res<<std::endl;
  std::cout<<jsta<<std::endl;
  std::cout<<jsta["STATE"].asString()<<std::endl;

  //LE_State_->setText(jsta["stateResponse"]["stateResult"][0].asString());
  currentState_=jsta["STATE"].asString();
  if (currentState_.compare("CREATED")==0)
    res=lcexec(" --daq-discover -v");
  res=lcexec(" --daq-setpar -v");
  res=lcexec(" --daq-serv -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;
  this->checkState();
}


/*void wlevbc::checkJobControl()
{
  std::string res=lcexec(" --jc-status -v");
  
  Json::Reader reader;
  Json::Value jsta,jdev1;
  bool parsingSuccessful = reader.parse(res,jsta);
  parsingSuccessful = reader.parse(jsta["jobStatusResponse"]["jobStatusResult"][0].asString(),jJC_);
  std::cout<<"JJC "<<jJC_<<std::endl;
}
*/
void wlevbc::checkLV()
{
  std::string res=lcexec(" --daq-lvsta -v");
  
  Json::Reader reader;
  Json::Value jsta,jdev1;
  bool parsingSuccessful = reader.parse(res,jsta);
  //parsingSuccessful = reader.parse(jsta["LVStatusResponse"]["LVStatusResult"][0].asString(),jLV_);
  jLV_=jsta["answer"]["LVSTATUS"];
  std::cout<<"JLV "<<jLV_<<std::endl;
}
void wlevbc::LVButtonHandler()
{
  
  if (isLVOn())
    std::string res=lcexec(" --daq-lvoff -v");
  else
    std::string res=lcexec(" --daq-lvon -v");
  checkState();
}

void wlevbc::InitialiseButtonHandler()
{
    std::string res=lcexec(" --daq-init -v");
    std::cout<<" SERVICE Called "<<res<<std::endl;
    this->checkState();
    
    PB_Configure_->enable();
    PB_Initialise_->disable();
    PB_Destroy_->enable();
}
void wlevbc::ConfigureButtonHandler()
{
  std::string res=lcexec(" --daq-conf -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;
  this->checkState();
  
  PB_Start_->enable();
  PB_Configure_->disable();
  PB_Destroy_->enable();
}
void wlevbc::StartButtonHandler()
{
  std::string res=lcexec(" --daq-start -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;
  this->checkState();
  PB_Stop_->enable();
  PB_Start_->disable();
  PB_Destroy_->enable();

  // Now fill the ELOG
  Wt::WDialog *dialog = new Wt::WDialog("Elog Message");
  dialog->setResizable(true);
  res=lcexec(" --daq-dbstatus -v");
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(res,jsta);
  Json::Value jdev1;
  jdev1.clear();
  jdev1=jsta["answer"];
  //parsingSuccessful = reader.parse(jsta["dbStatusResponse"]["dbStatusResult"][0].asString(),jdev1);
  Wt::WLabel *labelrun = new Wt::WLabel("Run",dialog->contents());
  Wt::WLineEdit *editrun = new Wt::WLineEdit(dialog->contents());editrun->setText(jdev1["run"].asString());
  editrun->disable();
  labelrun->setBuddy(editrun);
  Wt::WLabel *labelstate = new Wt::WLabel("State",dialog->contents());
  Wt::WLineEdit *editstate = new Wt::WLineEdit(dialog->contents());editstate->setText(jdev1["state"].asString());
  labelstate->setBuddy(editstate);
  editstate->disable();
  Wt::WLabel *labelbeam = new Wt::WLabel("Beam",dialog->contents());
  Wt::WLineEdit *editbeam = new Wt::WLineEdit(dialog->contents());editbeam->setText("D INDICE");
  labelbeam->setBuddy(editbeam);
  Wt::WLabel *labelenergy = new Wt::WLabel("Energy",dialog->contents());
  Wt::WLineEdit *editenergy = new Wt::WLineEdit(dialog->contents());editenergy->setText("DIESEL");
  labelenergy->setBuddy(editenergy);
  /*  Wt::WLabel *labelgas = new Wt::WLabel("GAS",dialog->contents());
  Wt::WLineEdit *editgas = new Wt::WLineEdit(dialog->contents());editgas->setText("PART");
  labelgas->setBuddy(editgas);
  Wt::WLabel *labelhv = new Wt::WLabel("HV",dialog->contents());
  Wt::WLineEdit *edithv = new Wt::WLineEdit(dialog->contents());editenergy->setText("LOW");
  labelhv->setBuddy(edithv);
  */

  Wt::WLabel *labelauthor = new Wt::WLabel("Author",dialog->contents());
  Wt::WLineEdit *editauthor = new Wt::WLineEdit(dialog->contents());editauthor->setText("COCO L'ASTICOT");
  labelauthor->setBuddy(editauthor);
  Wt::WLabel *labelcomment = new Wt::WLabel("Comment",dialog->contents());
  Wt::WLineEdit *editcomment = new Wt::WLineEdit(dialog->contents());editcomment->setText("Je ne mange pas de graines");
  labelcomment->setBuddy(editcomment);

  dialog->contents()->addStyleClass("form-group");

  Wt::WPushButton *ok = new Wt::WPushButton("OK", dialog->footer());
  ok->setDefault(true);

  Wt::WPushButton *cancel = new Wt::WPushButton("Cancel", dialog->footer());
  dialog->rejectWhenEscapePressed();

  /*
   * Accept the dialog
   */
  ok->clicked().connect(dialog,&Wt::WDialog::accept);

    /*
     * Reject the dialog
     */
  cancel->clicked().connect(dialog, &Wt::WDialog::reject);
    
    /*
     * Process the dialog result.
     */
    dialog->finished().connect(std::bind([=] () {
	if (dialog->result() == Wt::WDialog::Accepted)
	  {
	    std::cout<<"New log message: "<<editrun->text()<<std::endl;
	    std::cout<<"New log message: "<<editstate->text()<<std::endl;
	    std::cout<<"New log message: "<<editbeam->text()<<std::endl;
	    std::cout<<"New log message: "<<editenergy->text()<<std::endl;
	    std::cout<<"New log message: "<<editauthor->text()<<std::endl;
	    std::cout<<"New log message: "<<editcomment->text()<<std::endl;
	    char telog[1024];
	    memset(telog,0,1024);
	    sprintf(telog,"/usr/bin/elog -h lyosvn.in2p3.fr -d elog -s -l  %cILC Data Square Meter%c -p 443 -u acqilc RPC_2008 -a Auteur=%c%s%c -a Detecteur=%c%s%c -a Run=%c%s%c -a Faisceau=%c%s%c -a Energie=%c%s%c -a Evenements=%c%s%c -x      %c %s %c ",0x22,0x22, 0x22,editauthor->text().toUTF8().c_str(),0x22,0x22,editstate->text().toUTF8().c_str(),0x22,0x22,editrun->text().toUTF8().c_str(),0x22, 0x22,editbeam->text().toUTF8().c_str(),0x22, 0x22,editenergy->text().toUTF8().c_str(),0x22,0x22,"started",0x22,0x22,editcomment->text().toUTF8().c_str(),0x22);
	    std::cout<<telog<<std::endl;
	    std::string resl=exec((const char*) telog);
	    currentElog_=atoi(resl.substr(resl.find("ID=")+3,10).c_str());
	    currentSessionId_=this->sessionId();
	    // Publish SOR to monitoring
	    std::stringstream s,saction;
	    s<<"curl -X POST http://lyosdhcal12:8082/SDHCALRunControl/SOR ";
	    saction<<" -d run="<<editrun->text().toUTF8();
	    saction<<" -d detectorName=\""<<editstate->text().toUTF8()<<"\"";
	    saction<<" -d author=\""<<editauthor->text().toUTF8()<<"\"";
	    saction<<" -d beam=\""<<editbeam->text().toUTF8()<<"\"";
	    saction<<" -d energy=\""<<editenergy->text().toUTF8()<<"\"";
	    saction<<" -d description=\""<<editcomment->text().toUTF8()<<"\"";
	    s<<saction.str();
	    std::cout<<"Web command "<<s.str()<<std::endl;
	    std::string resm=exec(s.str().c_str());
	    std::cout<<resm<<std::endl;
	    
	  }
	else
	  std::cout<<"No location selected."<<std::endl;

	delete dialog;
    }));

    dialog->show();
}
void wlevbc::StopButtonHandler()
{
  std::string res=lcexec(" --daq-stop -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;
  this->checkState();
  if (this->sessionId().compare(currentSessionId_)==0 && currentElog_>0)
    {
      char telog[511];
      sprintf(telog,"/usr/bin/elog -h lyosvn.in2p3.fr -d elog -s -l  %cILC Data Square Meter%c -p 443 -u acqilc RPC_2008 -e %d  -a Evenements=%c%d%c ",0x22,0x22, currentElog_, 0x22,currentEvent_,0x22);
      std::cout<<telog<<std::endl;
      std::string resl=exec((const char*) telog);
      currentElog_=atoi(resl.substr(resl.find("ID=")+3,10).c_str());
    }
  // Publish EOR anywayto monitoring
  std::stringstream s,saction;
  s<<"curl -X POST http://lyosdhcal12:8082/SDHCALRunControl/EOR ";
  saction<<" -d events="<<currentEvent_;
  s<<saction.str();
  std::cout<<"Web command "<<s.str()<<std::endl;
  std::string resm=exec(s.str().c_str());
  std::cout<<resm<<std::endl;

      
    
  PB_Start_->enable();
  PB_Stop_->disable();
  PB_Stop_->disable();
  PB_Destroy_->enable();

}

void wlevbc::DestroyButtonHandler()
{
  std::string res=lcexec(" --daq-destroy -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;
  this->checkState();
  PB_Initialise_->enable();
  PB_Destroy_->disable();
  PB_Destroy_->disable();
}

void wlevbc::TResetButtonHandler()
{
  std::string res=lcexec(" --trig-reset ");
  this->checkState();
}
void wlevbc::TResumeButtonHandler()
{
  std::string res=lcexec(" --trig-resume ");
  this->checkState();
}
void wlevbc::TPauseButtonHandler()
{
  std::string res=lcexec(" --trig-pause ");
  this->checkState();
}
void wlevbc::HVSetButtonHandler()
{
  std::stringstream s;
  s<<" --slc-setvol --first="<<SB_FChan_->value()<<" --last="<<SB_LChan_->value()<<" --vol="<<SB_Vol_->value()<<" ";
  std::string res=lcexec(s.str().c_str());
  std::cout<<res<<std::endl;
  this->checkSlowControl();
}
void wlevbc::HVOnButtonHandler()
{
  std::stringstream s;
  s<<" --slc-hvon --first="<<SB_FChan_->value()<<" --last="<<SB_LChan_->value();
  std::string res=lcexec(s.str().c_str());
  std::cout<<res<<std::endl;

  this->checkSlowControl();
}
void wlevbc::HVOffButtonHandler()
{
  std::stringstream s;
  s<<" --slc-hvoff --first="<<SB_FChan_->value()<<" --last="<<SB_LChan_->value();
  std::string res=lcexec(s.str().c_str());
  std::cout<<res<<std::endl;

  this->checkSlowControl();
}


void wlevbc::updateMonitoringForm()
{
  return;
  //if (theClient_==NULL) return;
}
Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  /*
   * You could read information from the environment to decide whether
   * the user has permission to start a new application
   */
  return new wlevbc(env);
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

