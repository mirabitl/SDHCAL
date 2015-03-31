//
// C++ type definitions for package EVB.
// This file was generated automatically by yami4idl.
//

#ifndef YAMI4_IDL_EVB_H_INCLUDED
#define YAMI4_IDL_EVB_H_INCLUDED

#include <yami4-cpp/parameters.h>
#include <string>
#include <vector>

namespace yami
{
    class agent;
    class incoming_message;
}

#include "difhw.h"

namespace Evb
{

struct Config
{
    Config();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string Shmpath;
    int Numberoffragment;
    std::string Monitoringpath;
    int Monitoringsize;
    std::string Outputmode;
    std::string Outputpath;
    int Publishperiod;
};

struct Runconfig
{
    Runconfig();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    int Run;
    int Numberoffragment;
    std::string Dbstate;
};

struct Status
{
    Status();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string Evbstatus;
    bool RunValid;
    int Run;
    bool StarttimeValid;
    std::string Starttime;
    bool CompletedValid;
    int Completed;
    bool EventValid;
    int Event;
    bool DifidValid;
    std::vector<int> Difid;
    bool GtcValid;
    std::vector<int> Gtc;
    bool DtcValid;
    std::vector<int> Dtc;
    bool BcidValid;
    std::vector<int> Bcid;
};

class Statemachine
{
public:

    Statemachine(yami::agent & client_agent,
        const std::string & server_location, const std::string & object_name,
        int timeout = 0);

    void Initialise(const Config & Conf, Status & Res);
    void Start(const Runconfig & Runconf, Status & Res);
    void Stop(Status & Res);
    void Currentstatus(Status & Res);
    void Processdif(const Difhw::Data & Buf);

private:

    yami::agent & agent_;
    const std::string server_location_;
    const std::string object_name_;
    const std::size_t timeout_;
};

class StatemachineServer
{
public:

    virtual ~StatemachineServer() {}

    virtual void Initialise(const Config & Conf, Status & Res) = 0;
    virtual void Start(const Runconfig & Runconf, Status & Res) = 0;
    virtual void Stop(Status & Res) = 0;
    virtual void Currentstatus(Status & Res) = 0;
    virtual void Processdif(const Difhw::Data & Buf) = 0;

    void operator()(yami::incoming_message & im_);
};

} // namespace Evb

#endif // YAMI4_IDL_EVB_H_INCLUDED
