%module LYDaq
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "stdint.i"


 %{
using std::string;
#include "onedifhandler.h"
#include "difhw.h"
 using namespace Difhw;
#include "ccc.h"
 using namespace Ccc;
#include "zuplv.h"
 using namespace Zuplv;
#include "evb.h"
 using namespace Evb;
#include "YDaqManager.h"
 %}

%rename(Difhw_Config) Difhw::Config;
%rename(Difhw_Statemachine) Difhw::Statemachine;
%rename(Difhw_StatemachineServer) Difhw::StatemachineServer;
%rename(Ccc_Config) Ccc::Config;
%rename(Ccc_Status) Ccc::Status;
%rename(Ccc_Statemachine) Ccc::Statemachine;
%rename(Ccc_StatemachineServer) Ccc::StatemachineServer;
%rename(Zuplv_Config) Zuplv::Config;
%rename(Zuplv_Status) Zuplv::Status;
%rename(Zuplv_Statemachine) Zuplv::Statemachine;
%rename(Zuplv_StatemachineServer) Zuplv::StatemachineServer;
%rename(Evb_Config) Evb::Config;
%rename(Evb_Status) Evb::Status;
%rename(Evb_Statemachine) Evb::Statemachine;
%rename(Evb_StatemachineServer) Evb::StatemachineServer;
%include "YDaqManager.h"
%include "onedifhandler.h"
%include "difhw.h"
%include "ccc.h"
%include "zuplv.h"
%include "evb.h"



