#ifndef _Readout_Logger_
#define _Readout_Logger_
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;
static LoggerPtr _logDIF(Logger::getLogger("ReadoutDIF"));
static LoggerPtr _logCCC(Logger::getLogger("ReadoutCCC"));
static LoggerPtr _logFTDI(Logger::getLogger("ReadoutFTDI"));
#endif
