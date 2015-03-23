//
// C++ type definitions for package CCC.
// This file was generated automatically by yami4idl.
//

#ifndef YAMI4_IDL_CCC_H_INCLUDED
#define YAMI4_IDL_CCC_H_INCLUDED

#include <yami4-cpp/parameters.h>
#include <string>
#include <vector>

namespace yami
{
    class agent;
    class incoming_message;
}

namespace Ccc
{

struct Config
{
    Config();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string Serial;
};

struct Registeraccess
{
    Registeraccess();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    int Address;
    int Cccregister;
};

struct Status
{
    Status();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    int Cccstatus;
    bool CccregisterValid;
    int Cccregister;
};

class Statemachine
{
public:

    Statemachine(yami::agent & client_agent,
        const std::string & server_location, const std::string & object_name,
        int timeout = 0);

    void Initialise(Status & Res);
    void Configure(Status & Res);
    void Difreset(Status & Res);
    void Bcidreset(Status & Res);
    void Startacquisitionauto(Status & Res);
    void Ramfullext(Status & Res);
    void Trigext(Status & Res);
    void Stopacquisition(Status & Res);
    void Startsingleacquisition(Status & Res);
    void Pulselemo(Status & Res);
    void Trigger(Status & Res);
    void Cccreset(Status & Res);
    void Spillon(Status & Res);
    void Spilloff(Status & Res);
    void Pausetrigger(Status & Res);
    void Resumetrigger(Status & Res);
    void Writeregister(const Registeraccess & Ra, Status & Res);
    void Readregister(const Registeraccess & Ra, Status & Res);
    void Open(const Config & Conf);

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
    virtual void Configure(Status & Res) = 0;
    virtual void Difreset(Status & Res) = 0;
    virtual void Bcidreset(Status & Res) = 0;
    virtual void Startacquisitionauto(Status & Res) = 0;
    virtual void Ramfullext(Status & Res) = 0;
    virtual void Trigext(Status & Res) = 0;
    virtual void Stopacquisition(Status & Res) = 0;
    virtual void Startsingleacquisition(Status & Res) = 0;
    virtual void Pulselemo(Status & Res) = 0;
    virtual void Trigger(Status & Res) = 0;
    virtual void Cccreset(Status & Res) = 0;
    virtual void Spillon(Status & Res) = 0;
    virtual void Spilloff(Status & Res) = 0;
    virtual void Pausetrigger(Status & Res) = 0;
    virtual void Resumetrigger(Status & Res) = 0;
    virtual void Writeregister(const Registeraccess & Ra, Status & Res) = 0;
    virtual void Readregister(const Registeraccess & Ra, Status & Res) = 0;
    virtual void Open(const Config & Conf) = 0;

    void operator()(yami::incoming_message & im_);
};

} // namespace Ccc

#endif // YAMI4_IDL_CCC_H_INCLUDED
