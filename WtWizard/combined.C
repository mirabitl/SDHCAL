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
#include <json/json.h>
//using namespace Wt;
 #define STEP printf("%s %d\n",__FUNCTION__,__LINE__)
/*
 * A simple hello world application class which demonstrates how to react
 * to events, read input, and give feed-back.
 */
class combined : public Wt::WApplication
{
public:
  combined(const Wt::WEnvironment& env);

private:
  uint32_t theRunNumber_;



  // Used widgets
  Wt::WPushButton *PB_Initialise_;
  Wt::WPushButton *PB_Configure_;
  Wt::WPushButton *PB_Start_;
  Wt::WPushButton *PB_Stop_;
  Wt::WPushButton *PB_Destroy_;
  Wt::WPushButton *PB_Status_;


  Wt::WPushButton *PB_TReset_;
  Wt::WPushButton *PB_TResume_;
  Wt::WPushButton *PB_TPause_;

  
  Wt::WContainerWidget *container_;
  Wt::WTabWidget *tabW_; 
  Wt::WText* tDaqStatus_;
  std::string currentState_;
  Json::Value jLV_;
  Json::Value jAvail_;
  uint32_t currentRun_,currentEvent_,currentElog_;
  std::string currentSessionId_;
  std::string _ecalStatus;
  void Quit();
  void buildDaqForm(Wt::WPanel *wb);
  void fillDaqStatus(Wt::WPanel *wb);
  void InitialiseButtonHandler();
  void ConfigureButtonHandler();
  void StartButtonHandler();
  void StopButtonHandler();
  void DestroyButtonHandler();
  void TResetButtonHandler();
  void TResumeButtonHandler();
  void TPauseButtonHandler();

  void updateMonitoringForm();

  void toggleButtons();
  std::string daqStatus();
  void checkState();
  void checkLV();
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


combined::combined(const Wt::WEnvironment& env)
  : Wt::WApplication(env),currentElog_(0),currentSessionId_(""),_ecalStatus("UNKNOWN")
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
      jAvail_["DAQ"].asString().compare("DEAD")==0 ||
      jAvail_["ECAL"].asString().compare("DEAD")==0)
    {
      std::stringstream ss;
      ss<<"<p>Missing web services</p>";
      ss<<"<p> JOB Control state :"<< jAvail_["JOB"].asString()<<"</p>";
      ss<<"<p> HV Control state :"<< jAvail_["SLOW"].asString()<<"</p>";
      ss<<"<p> SDHCAL DAQ state :"<< jAvail_["DAQ"].asString()<<"</p>";
      ss<<"<p> ECAL DAQ state :"<< jAvail_["ECAL"].asString()<<"</p>";
      Wt::WMessageBox *messageBox = new Wt::WMessageBox
	("Error",ss.str(),
	 Wt::Information, Wt::Yes);
      //"<p>Missing web services</p>"
      //	 "<p>Please start it and reload the page </p>",

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
 
  setTitle("Combined CALICE ECAL-SDHCAL Beamtest");                               // application title
  container_ = new Wt::WContainerWidget(root());
  container_->setStyleClass("bootstrap");
  tabW_ = new Wt::WTabWidget(container_);
 

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

  b1->clicked().connect(this, &combined::Quit);
  tabW_->currentChanged().connect(this,&combined::updateMonitoringForm);

}

void combined::Quit()
{
  /*
   * Update the text, using text input into the nameEdit_ field.
   */
  std::string sh="http://"+this->environment().hostName()+"/test.html";		
  this->redirect(sh);

  this->quit();
}



std::string combined::daqStatus()
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
  os<<"<h4> SDHCAL State :"<<jsta["STATE"].asString()<<"</h4>";
  res=lcexec(" --ecal-state -v");
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  os<<"<h4> ECAL State :"<<jsta["STATE"].asString()<<"</h4>";
  std::replace( _ecalStatus.begin(), _ecalStatus.end(), '<', ':');
  std::replace( _ecalStatus.begin(), _ecalStatus.end(), '>', ':');
  os<<"<h4> ECAL Status :</h4> <pre>"<<_ecalStatus<<"</pre>";
  //Event builder
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
      os<<"<p><b>SDHCAL Event:</b> "<<jdev1["event"].asString()<<"</p>";
      }
      catch(...)
	{
	}
    }

  // Ecal status
  res=lcexec(" --ecal-currentspill -v");
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  jdev1.clear();
  //parsingSuccessful = reader.parse(jsta["shmStatusResponse"]["shmStatusResult"][0].asString(),jdev1);
  jdev1=jsta["answer"];
  os<<"<p><b>Ecal Spill:</b> "<<jdev1["CURRENTSPILL"].asUInt()<<"</p>";
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
  return os.str();
}
void combined::fillDaqStatus(Wt::WPanel *wp)
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
void combined::buildDaqForm(Wt::WPanel *wp)
{
  Wt::WContainerWidget *wb = new Wt::WContainerWidget();
  wp->setCentralWidget(wb);
  Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
  vbox->setContentsMargins(12,12,12,12);
  wb->setLayout(vbox);
  //wb->resize(200,200);

  Wt::WHBoxLayout *hbFSM = new Wt::WHBoxLayout();



  PB_Initialise_= new Wt::WPushButton("Initialise", wb);
  PB_Configure_= new Wt::WPushButton("Configure", wb);//PB_Configure_->disable();
  PB_Start_= new Wt::WPushButton("Start", wb);//PB_Start_->disable();
  PB_Stop_= new Wt::WPushButton("Stop", wb);//PB_Stop_->disable();
  PB_Destroy_= new Wt::WPushButton("Destroy", wb);//PB_Destroy_->disable();
  PB_Status_= new Wt::WPushButton("Status", wb);//PB_Destroy_->disable();


  hbFSM->addWidget(PB_Initialise_);  // show some text
  hbFSM->addWidget(PB_Configure_);  // show some text
  hbFSM->addWidget(PB_Start_);  // show some text
  hbFSM->addWidget(PB_Stop_);  // show some text
  hbFSM->addWidget(PB_Destroy_);  // show some text
  hbFSM->addWidget(PB_Status_);  // show some text
  vbox->addLayout(hbFSM);

  Wt::WHBoxLayout *hbinfo = new Wt::WHBoxLayout();
  //  Wt::WGroupBox *groupBox = new Wt::WGroupBox();
  // LE_State_ = new Wt::WLineEdit();                     // allow text input
  // LE_State_->setText("Destroyed");
  // LE_State_->disable();

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

  PB_Initialise_->clicked().connect(this,&combined::InitialiseButtonHandler);

  PB_Start_->clicked().connect(this,&combined::StartButtonHandler);
  PB_Configure_->clicked().connect(this,&combined::ConfigureButtonHandler);
  PB_Stop_->clicked().connect(this,&combined::StopButtonHandler);
  PB_Destroy_->clicked().connect(this,&combined::DestroyButtonHandler);
  PB_Status_->clicked().connect(this,&combined::checkState);

  PB_TReset_->clicked().connect(this,&combined::TResetButtonHandler);
  PB_TResume_->clicked().connect(this,&combined::TResumeButtonHandler);
  PB_TPause_->clicked().connect(this,&combined::TPauseButtonHandler);

}
//////////////////////////////////////////////////////////////////////////////////



void combined::checkState()
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


  this->toggleButtons();
  tDaqStatus_->setText(this->daqStatus());
 
}
void combined::toggleButtons()
{
  if (currentState_.compare("DISCOVERED")==0 )
    {

      
      PB_Initialise_->disable();
      PB_Configure_->disable();
      PB_Start_->disable();
      PB_Stop_->disable();
      PB_Destroy_->disable();
      Wt::WMessageBox *messageBox = new Wt::WMessageBox
	("Error",
	 "<p>SDHCAL DAQ should be PREPARED</p>"
	 "<p>DO it on SDHCAL WEB Page and retry </p>",
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
  if (currentState_.compare("PREPARED")==0 )
    {

      PB_Initialise_->enable();
      PB_Configure_->disable();
      PB_Start_->disable();
      PB_Stop_->disable();
      PB_Destroy_->disable();
      
    }
  if (currentState_.compare("INITIALISED")==0 )
    {
   
      PB_Initialise_->disable();
      PB_Configure_->enable();
      PB_Start_->disable();
      PB_Stop_->disable();
      PB_Destroy_->enable();
      
    }
  if (currentState_.compare("CONFIGURED")==0 )
    {

      PB_Initialise_->disable();
      PB_Configure_->enable();
      PB_Start_->enable();
      PB_Stop_->disable();
      PB_Destroy_->enable();
      
    }
  if (currentState_.compare("RUNNING")==0 )
    {

      PB_Initialise_->disable();
      PB_Configure_->disable();
      PB_Start_->disable();
      PB_Stop_->enable();
      PB_Destroy_->disable();
      
    }
  checkLV();
  if (!isLVOn())
    {
      PB_Initialise_->disable();
      PB_Configure_->disable();
      PB_Start_->disable();
      PB_Stop_->disable();
      PB_Destroy_->disable();
      Wt::WMessageBox *messageBox = new Wt::WMessageBox
	("Error",
	 "<p>SDHCAL is LV OFF</p>"
	 "<p>Turn it on SDHCAL WEB Page and retry </p>",
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
}





void combined::checkLV()
{
  std::string res=lcexec(" --daq-lvsta -v");
  
  Json::Reader reader;
  Json::Value jsta,jdev1;
  bool parsingSuccessful = reader.parse(res,jsta);
  //parsingSuccessful = reader.parse(jsta["LVStatusResponse"]["LVStatusResult"][0].asString(),jLV_);
  jLV_=jsta["answer"]["LVSTATUS"];
  std::cout<<"JLV "<<jLV_<<std::endl;
}

void combined::InitialiseButtonHandler()
{
    std::string res=lcexec(" --daq-init -v");
    std::cout<<" SERVICE Called "<<res<<std::endl;
    this->checkState();
    
    PB_Configure_->enable();
    PB_Initialise_->disable();
    PB_Destroy_->enable();
}
void combined::ConfigureButtonHandler()
{
  std::string res=lcexec(" --daq-conf -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;
  res=lcexec(" --ecal-conf -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;
  Json::Reader reader;
  Json::Value jsta,jdev1;
  jsta.clear();
  bool parsingSuccessful = reader.parse(res,jsta);
  jdev1.clear();
  //parsingSuccessful = reader.parse(jsta["shmStatusResponse"]["shmStatusResult"][0].asString(),jdev1);
  jdev1=jsta["content"]["answer"];
  std::cout<<jsta<<std::endl;
  std::cout<<"DEV1 "<<jdev1<<std::endl;
  std::cout<<"DEV2 "<<jdev1["ECALSTATUS"]<<std::endl;
  _ecalStatus=jdev1["ECALSTATUS"].asString();
  
  this->checkState();
  
  PB_Start_->enable();
  PB_Configure_->enable();
  PB_Destroy_->enable();
}
void combined::StartButtonHandler()
{
  std::string res=lcexec(" --daq-start -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;


  res=lcexec(" --daq-dbstatus -v");
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(res,jsta);
  Json::Value jdev1;
  jdev1.clear();
  jdev1=jsta["answer"];
  int run=jdev1["run"].asUInt();
  std::stringstream s;
  s<<" --ecal-start --run="<<jdev1["run"].asUInt();
  res=lcexec(s.str().c_str());

  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  jdev1.clear();
  //parsingSuccessful = reader.parse(jsta["shmStatusResponse"]["shmStatusResult"][0].asString(),jdev1);
  jdev1=jsta["content"]["answer"];
  _ecalStatus=jdev1["ECALSTATUS"].asString();
  
  this->checkState();
  
  PB_Stop_->enable();
  PB_Start_->disable();
  PB_Destroy_->enable();
  res=lcexec(" --daq-dbstatus -v");
  jsta.clear();
  parsingSuccessful = reader.parse(res,jsta);
  jdev1.clear();
  jdev1=jsta["answer"];
  // Now fill the ELOG
  Wt::WDialog *dialog = new Wt::WDialog("Elog Message");
  dialog->setResizable(true);
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
	    s<<"curl -X POST http://lyoac29:8082/SDHCALRunControl/SOR ";
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
void combined::StopButtonHandler()
{
  std::string res=lcexec(" --daq-stop -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;
  res=lcexec(" --ecal-stop -v");
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
  s<<"curl -X POST http://lyoac29:8082/SDHCALRunControl/EOR ";
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

void combined::DestroyButtonHandler()
{
  std::string res=lcexec(" --daq-destroy -v");
  std::cout<<" SERVICE Called "<<res<<std::endl;
  this->checkState();
  PB_Initialise_->enable();
  PB_Destroy_->disable();
  PB_Destroy_->disable();
}

void combined::TResetButtonHandler()
{
  std::string res=lcexec(" --trig-reset ");
  this->checkState();
}
void combined::TResumeButtonHandler()
{
  std::string res=lcexec(" --trig-resume ");
  this->checkState();
}
void combined::TPauseButtonHandler()
{
  std::string res=lcexec(" --trig-pause ");
  this->checkState();
}


void combined::updateMonitoringForm()
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
  return new combined(env);
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

