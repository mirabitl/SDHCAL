//
// C++ type definitions for package ZUP.
// This file was generated automatically by yami4idl.
//

#ifndef YAMI4_IDL_ZUP_H_INCLUDED
#define YAMI4_IDL_ZUP_H_INCLUDED

#include <yami4-cpp/parameters.h>
#include <string>
#include <vector>

namespace yami
{
    class agent;
    class incoming_message;
}

namespace Zup
{

struct Config
{
    Config();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string Serial;
};

struct Status
{
    Status();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string Zupstatus;
    double Voltage;
    double Current;
};

class Statemachine
{
public:

    Statemachine(yami::agent & client_agent,
        const std::string & server_location, const std::string & object_name,
        int timeout = 0);

    void Check(Status & Res);
    void On(Status & Res);
    void Off(Status & Res);
    void Initialise(const Config & Conf);

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

    virtual void Check(Status & Res) = 0;
    virtual void On(Status & Res) = 0;
    virtual void Off(Status & Res) = 0;
    virtual void Initialise(const Config & Conf) = 0;

    void operator()(yami::incoming_message & im_);
};

} // namespace Zup

#endif // YAMI4_IDL_ZUP_H_INCLUDED
