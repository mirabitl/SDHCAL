//
// C++ type definitions for package DIFHW.
// This file was generated automatically by yami4idl.
//

#ifndef YAMI4_IDL_DIFHW_H_INCLUDED
#define YAMI4_IDL_DIFHW_H_INCLUDED

#include <yami4-cpp/parameters.h>
#include <string>
#include <vector>

namespace yami
{
    class agent;
    class incoming_message;
}

#include "odb.h"

namespace Difhw
{

struct Config
{
    Config();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    int Trigger;
    std::string Dbstate;
};

struct Difstatus
{
    Difstatus();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::vector<int> Id;
    std::vector<int> Status;
    std::vector<std::string> Debug;
    bool GtcValid;
    std::vector<int> Gtc;
    bool BcidValid;
    std::vector<int> Bcid;
};

struct Scanstatus
{
    Scanstatus();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::vector<int> Diflist;
};

struct Data
{
    Data();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    int Difid;
    int Gtc;
    int Bcid;
    std::vector<char> Payload;
};

class Statemachine
{
public:

    Statemachine(yami::agent & client_agent,
        const std::string & server_location, const std::string & object_name,
        int timeout = 0);

    void Scan(Scanstatus & Res);
    void Registerdb(const Config & Conf, Difstatus & Res);
    void Loadslowcontrol(Difstatus & Res);
    void Initialise(const Scanstatus & Conf, Difstatus & Res);
    void Start(Difstatus & Res);
    void Stop(Difstatus & Res);
    void Destroy(Difstatus & Res);
    void Processslowcontrol(const Odb::Dbbuffer & Buf);

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

    virtual void Scan(Scanstatus & Res) = 0;
    virtual void Registerdb(const Config & Conf, Difstatus & Res) = 0;
    virtual void Loadslowcontrol(Difstatus & Res) = 0;
    virtual void Initialise(const Scanstatus & Conf, Difstatus & Res) = 0;
    virtual void Start(Difstatus & Res) = 0;
    virtual void Stop(Difstatus & Res) = 0;
    virtual void Destroy(Difstatus & Res) = 0;
    virtual void Processslowcontrol(const Odb::Dbbuffer & Buf) = 0;

    void operator()(yami::incoming_message & im_);
};

} // namespace Difhw

#endif // YAMI4_IDL_DIFHW_H_INCLUDED
