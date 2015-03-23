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

namespace evb
{

struct config
{
    config();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string shmpath;
    int numberoffragment;
    std::string monitoringpath;
    int monitoringsize;
    std::string outputmode;
    std::string outputpath;
    int publishperiod;
};

struct status
{
    status();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string evbstatus;
    bool run_valid;
    int run;
    bool starttime_valid;
    std::string starttime;
    bool completed_valid;
    int completed;
    bool event_valid;
    int event;
};

class statemachine
{
public:

    statemachine(yami::agent & client_agent,
        const std::string & server_location, const std::string & object_name,
        int timeout = 0);

    void initialise(const config & conf, status & res);
    void start(status & res);
    void stop(status & res);

private:

    yami::agent & agent_;
    const std::string server_location_;
    const std::string object_name_;
    const std::size_t timeout_;
};

class statemachine_server
{
public:

    virtual ~statemachine_server() {}

    virtual void initialise(const config & conf, status & res) = 0;
    virtual void start(status & res) = 0;
    virtual void stop(status & res) = 0;

    void operator()(yami::incoming_message & im_);
};

} // namespace evb

#endif // YAMI4_IDL_EVB_H_INCLUDED
