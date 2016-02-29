#ifndef _Ctrl_Logger_
#define _Ctrl_Logger_
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;
static LoggerPtr _logCtrl(Logger::getLogger("CtrlDAQ"));
static LoggerPtr _logWriter(Logger::getLogger("CtrlWriter"));
#endif
