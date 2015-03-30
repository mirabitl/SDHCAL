//
// C++ type definitions for package ODB.
// This file was generated automatically by yami4idl.
//

#ifndef YAMI4_IDL_ODB_H_INCLUDED
#define YAMI4_IDL_ODB_H_INCLUDED

#include <yami4-cpp/parameters.h>
#include <string>
#include <vector>

namespace yami
{
    class agent;
    class incoming_message;
}

namespace Odb
{

struct Config
{
    Config();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string Dbstate;
};

struct Dbbuffer
{
    Dbbuffer();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    int Difid;
    int Nasic;
    std::vector<char> Payload;
};

struct Status
{
    Status();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string Oraclestatus;
};

class Statemachine
{
public:

    Statemachine(yami::agent & client_agent,
        const std::string & server_location, const std::string & object_name,
        int timeout = 0);

    void Initialise(Status & Res);
    void Download(const Config & Conf, Status & Res);
    void Dispatch(Status & Res);

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

    virtual void Initialise(Status & Res) = 0;
    virtual void Download(const Config & Conf, Status & Res) = 0;
    virtual void Dispatch(Status & Res) = 0;

    void operator()(yami::incoming_message & im_);
};

} // namespace Odb

#endif // YAMI4_IDL_ODB_H_INCLUDED
