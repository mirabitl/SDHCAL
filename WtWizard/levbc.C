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
  Wt::WCheckBox *CB_DIF_;
  Wt::WSpinBox  *SB_CCCPort_;
  Wt::WSpinBox  *SB_DIFPort_;
  Wt::WSpinBox  *SB_NumberOfDIFs_;
  Wt::WSpinBox  *SB_DefaultRunNumber_;
  Wt::WLineEdit *LE_State_;
  Wt::WPushButton *PB_Service_;
  Wt::WPushButton *PB_LV_;
  Wt::WPushButton *PB_Initialise_;
  Wt::WPushButton *PB_Configure_;
  Wt::WPushButton *PB_Start_;
  Wt::WPushButton *PB_Stop_;
  Wt::WPushButton *PB_Destroy_;
  Wt::WPushButton *PB_RefreshStatus_;
  Wt::WPushButton *PB_Status_;
  Wt::WPushButton *PB_HVStatus_;
  Wt::WTable *TB_DIFStatus_;
  Wt::WTable *TB_TrigStatus_;
  Wt::WLineEdit *LE_RunStatus_;
  Wt::WLineEdit *LE_RunNumber_;
  Wt::WLineEdit *LE_EventNumber_;
  Wt::WContainerWidget *container_;
  Wt::WTabWidget *tabW_; 
  Wt::WText* tDaqStatus_;
  Wt::WText* tHVStatus_;
  
  std::string currentState_;
  void Quit();
  void buildDaqForm(Wt::WPanel *wb);
  void buildHVForm(Wt::WPanel *wb);
  void fillDaqStatus(Wt::WPanel *wb);
  void fillHVStatus(Wt::WPanel *wb);
  void ServiceButtonHandler();
  void LVButtonHandler();
  void checkState();
  void checkSlowControl();
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
  void toggleButtons();
  std::string daqStatus();
  std::string hvStatus();
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

  Wt::WApplication::instance()->useStyleSheet("bootstrap/css/bootstrap.min.css");
  std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --available -v");
  std::cout<<"Youpi Ya "<<res<<std::endl;
  setTitle("Standalone Data Acquisition");                               // application title
  container_ = new Wt::WContainerWidget(root());
  container_->setStyleClass("bootstrap");
  tabW_ = new Wt::WTabWidget(container_);
 




  Wt::WPanel *groupBoxHV = new Wt::WPanel();
  groupBoxHV->setTitle("HV Control");
  groupBoxHV->setCollapsible(true);
  
  tabW_->addTab(groupBoxHV,"HV Control");//, Wt::WTabWidget::PreLoading);

  groupBoxHV->addStyleClass("centered-example");
  buildHVForm(groupBoxHV);

  
  //Wt::WGroupBox *groupBoxDaq = new Wt::WGroupBox("Daq Control");
  Wt::WPanel *groupBoxDaq = new Wt::WPanel();
  groupBoxDaq->setTitle("Daq Control");
  groupBoxDaq->setCollapsible(true);
  
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


std::string levbc::hvStatus()
{
  std::stringstream os;
  std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --slc-pt -v");

  //if (theClient_==NULL) return;
  Json::Reader reader;
  Json::Value jsta;
  
  os<<"<h4> Environment </h4>"<<std::endl;
  
  jsta.clear();
  bool parsingSuccessful = reader.parse(res,jsta);
  //std::cout<<jsta<<std::endl;
  Json::Value jdev1;
  parsingSuccessful = reader.parse(jsta["PTResponse"]["PTResult"][0].asString(),jdev1);
  //std::cout<<jdev1;
  os<<"<table style=\"width:100%\">";
  os<<"<tr><th style=\"text-align:left\">Pression</th><th style=\"text-align:left\">Temperature</th> </tr>";

  os<<"<tr><td>"<<jdev1["P"].asFloat()<<"</td>";
  os<<"<td>"<<jdev1["T"].asFloat()<<"</td></tr>";
  os<<"</table>";


  res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --slc-hvstatus -v");
  //std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  //  std::cout<<jsta;
  parsingSuccessful = reader.parse(jsta["hvStatusResponse"]["hvStatusResult"][0].asString(),jdev1);
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


std::string levbc::daqStatus()
{
  STEP;
  std::stringstream os;
  std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-state -v");

  //if (theClient_==NULL) return;
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(res,jsta);
  currentState_=jsta["stateResponse"]["stateResult"][0].asString();
  os<<"<h4> State :"<<jsta["stateResponse"]["stateResult"][0].asString()<<"</h4>";
  //Event builder
  STEP;
  os<<"<h4> Low Voltage </h4>"<<std::endl;
  res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-lvsta -v");
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  Json::Value jdev1;
  std::cout<<"JSTA"<<jsta;
  parsingSuccessful = reader.parse(jsta["LVStatusResponse"]["LVStatusResult"][0].asString(),jdev1);
  std::cout<<jdev1<<jdev1.empty()<<jsta["LVStatusResponse"]["LVStatusResult"][0];
  STEP;
  if (jsta["LVStatusResponse"]["LVStatusResult"][0].asString().compare("null")!=0)
    {
      os<<"<table style=\"width:100%\">";
      os<<"<tr><th style=\"text-align:left\">VSET</th><th style=\"text-align:left\">VOUT</th> <th style=\"text-align:left\">IOUT</th></tr>";
      
      os<<"<tr><td>"<<jdev1["vset"].asFloat()<<"</td>";
      os<<"<td>"<<jdev1["vout"].asFloat()<<"</td>";
      os<<"<td>"<<jdev1["iout"].asFloat()<<"</td></tr>";
      os<<"</table>";
    }

  STEP;
  
  res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-evb -v");
  std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  jdev1.clear();
  parsingSuccessful = reader.parse(jsta["shmStatusResponse"]["shmStatusResult"][0].asString(),jdev1);
  std::cout<<currentState_<<currentState_.compare("INITIALISED")<<std::endl;
  std::cout<<jdev1<<std::endl;
  STEP;
  if (parsingSuccessful && !jdev1.empty())
    {
      try {
      os<<"<h4> Event Builder:</h4>";
      os<<"<p><b>Run:</b> "<<jdev1["run"].asString()<<"</p>";
      os<<"<p><b>Event:</b> "<<jdev1["event"].asString()<<"</p>";
      }
      catch(...)
	{
	}
    }
  STEP;
  os<<"<h4> Trigger Status MDCC:</h4>"<<std::endl;
  res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --trig-status -v");
  //std::cout<<res<<std::endl;
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  //std::cout<<jsta;
  parsingSuccessful = reader.parse(jsta["triggerStatusResponse"]["triggerStatusResult"][0].asString(),jdev1);
  if (parsingSuccessful && !jdev1["spill"].empty())
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
  STEP;
  if (!CB_DIF_->isChecked()) return os.str();
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
void levbc::fillDaqStatus(Wt::WPanel *wp)
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
void levbc::buildDaqForm(Wt::WPanel *wp)
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
  hbFSM->addWidget(PB_LV_);  // show some text
  hbFSM->addWidget(PB_Initialise_);  // show some text
  hbFSM->addWidget(PB_Configure_);  // show some text
  hbFSM->addWidget(PB_Start_);  // show some text
  hbFSM->addWidget(PB_Stop_);  // show some text
  hbFSM->addWidget(PB_Destroy_);  // show some text
  hbFSM->addWidget(PB_Status_);  // show some text
  hbFSM->addWidget(CB_DIF_);  // show some text
  vbox->addLayout(hbFSM);
  Wt::WPanel *pDaqStatus = new Wt::WPanel();
  pDaqStatus->setTitle("Daq Status");
  pDaqStatus->setCollapsible(true);
  fillDaqStatus(pDaqStatus);
  vbox->addWidget(pDaqStatus);
  
  // Wt::WHBoxLayout *hbinfo = new Wt::WHBoxLayout();
  // Wt::WGroupBox *groupBox = new Wt::WGroupBox();
  // LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();
  this->checkState();
  // groupBox->addWidget(LE_State_);
  // hbinfo->addWidget(groupBox);
  // vbox->addLayout(hbinfo);
  PB_Service_->clicked().connect(this,&levbc::ServiceButtonHandler);
  PB_LV_->clicked().connect(this,&levbc::LVButtonHandler);
  PB_Initialise_->clicked().connect(this,&levbc::InitialiseButtonHandler);

  PB_Start_->clicked().connect(this,&levbc::StartButtonHandler);
  PB_Configure_->clicked().connect(this,&levbc::ConfigureButtonHandler);
  PB_Stop_->clicked().connect(this,&levbc::StopButtonHandler);
  PB_Destroy_->clicked().connect(this,&levbc::DestroyButtonHandler);
  PB_Status_->clicked().connect(this,&levbc::checkState);


}
void levbc::fillHVStatus(Wt::WPanel *wp)
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

void levbc::buildHVForm(Wt::WPanel *wp)
{
  STEP;
  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(12,12,12,12);
  wb->setLayout(vbox);

  //wb->resize(200,200);

  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();
  STEP;


  /*  PB_Service_= new Wt::WPushButton("Service", wb);
  PB_LV_= new Wt::WPushButton("LV Switch", wb);
  PB_Initialise_= new Wt::WPushButton("Initialise", wb);
  PB_Configure_= new Wt::WPushButton("Configure", wb);//PB_Configure_->disable();
  PB_Start_= new Wt::WPushButton("Start", wb);//PB_Start_->disable();
  PB_Stop_= new Wt::WPushButton("Stop", wb);//PB_Stop_->disable();
  PB_Destroy_= new Wt::WPushButton("Destroy", wb);//PB_Destroy_->disable();
  */
  PB_HVStatus_= new Wt::WPushButton("Status", wb);//PB_Destroy_->disable();
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
  hbFSM->addWidget(PB_HVStatus_);  // show some text
  // hbFSM->addWidget(CB_DIF_);  // show some text
  vbox->addLayout(hbFSM);
  STEP;
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
  PB_Service_->clicked().connect(this,&levbc::ServiceButtonHandler);
  PB_LV_->clicked().connect(this,&levbc::LVButtonHandler);
  PB_Initialise_->clicked().connect(this,&levbc::InitialiseButtonHandler);

  PB_Start_->clicked().connect(this,&levbc::StartButtonHandler);
  PB_Configure_->clicked().connect(this,&levbc::ConfigureButtonHandler);
  PB_Stop_->clicked().connect(this,&levbc::StopButtonHandler);
  PB_Destroy_->clicked().connect(this,&levbc::DestroyButtonHandler);
  */
  PB_HVStatus_->clicked().connect(this,&levbc::checkSlowControl);
  STEP;

}
void levbc::checkSlowControl()
{
  tHVStatus_->setText(this->hvStatus());
}
void levbc::checkState()
{
  std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-state -v");
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(res,jsta);
  //LE_State_->setText(jsta["stateResponse"]["stateResult"][0].asString());
  currentState_=jsta["stateResponse"]["stateResult"][0].asString();
  this->toggleButtons();
  tDaqStatus_->setText(this->daqStatus());
 
}
void levbc::toggleButtons()
{
  if (currentState_.compare("DISCOVERED")==0 || currentState_.compare("PREPARED")==0 )
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
      PB_Service_->disable();
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
}
void levbc::ServiceButtonHandler()
{
  try {

    std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-setpar -v");
    res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-serv -v");
    std::cout<<" SERVICE Called "<<res<<std::endl;
    this->checkState();
    //PB_Configure_->enable();
  }
  catch(...)
    {
      LE_State_->setText("Failure not Initialised");
    }

  



}
void levbc::LVButtonHandler()
{
  try {

    std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-lvsta -v");
    std::cout<<"RC" <<res<<std::endl;
    Json::Reader reader;
    Json::Value jsta,jdev1;
    bool parsingSuccessful = reader.parse(res,jsta);
    parsingSuccessful = reader.parse(jsta["LVStatusResponse"]["LVStatusResult"][0].asString(),jdev1);
    std::cout<<"JDEV1 "<<jdev1<<std::endl;
    if (jdev1["vout"].asFloat()<1)
      res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-lvon -v");
    else
      res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-lvoff -v");
    res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-lvsta -v");
    parsingSuccessful = reader.parse(res,jsta);
    parsingSuccessful = reader.parse(jsta["LVStatusResponse"]["LVStatusResult"][0].asString(),jdev1);
    std::cout<<"JDEV1 "<<jdev1<<std::endl;
    if (jdev1["vout"].asFloat()<1)
      PB_LV_->setStyleClass("btn-danger");
    else
      PB_LV_->setStyleClass("btn-success");
    
    //PB_Configure_->enable();
  }
  catch(...)
    {
      LE_State_->setText("Failure not Initialised");
    }

  



}

void levbc::InitialiseButtonHandler()
{
  try {
    std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-init -v");
    std::cout<<" SERVICE Called "<<res<<std::endl;
    this->checkState();
    
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


   std::cout<<"Fin de l'initialise"<<this->sessionId()<<std::endl;  
  theRunInfo_=NULL;
}
void levbc::ConfigureButtonHandler()
{
  try {
      std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-conf -v");
    std::cout<<" SERVICE Called "<<res<<std::endl;
    this->checkState();

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
    std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-start -v");
    std::cout<<" SERVICE Called "<<res<<std::endl;
    this->checkState();
    PB_Stop_->enable();
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
    std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-stop -v");
    std::cout<<" SERVICE Called "<<res<<std::endl;
    this->checkState();

    PB_Start_->enable();
    PB_Stop_->disable();
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


  PB_Destroy_->enable();

}
void levbc::HaltAction()
{
  this->StopAction();
}

void levbc::DestroyButtonHandler()
{
  try {

    std::string res=exec("cd /home/mirabito/SDHCAL/levbdim_daq; . ./levbdimrc; ./lc.py --daq-destroy -v");
    std::cout<<" SERVICE Called "<<res<<std::endl;
    this->checkState();

    PB_Initialise_->enable();
    PB_Destroy_->disable();
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
  Wt::WContainerWidget *container = new Wt::WContainerWidget(wb);

 Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  //vbox->setContentsMargins(2,2,2,2);
 container->setLayout(vbox);
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

void levbc::updateMonitoringForm()
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

