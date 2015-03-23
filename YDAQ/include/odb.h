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

namespace odb
{

struct config
{
    config();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::string dbstate;
};

struct dbbuffer
{
    dbbuffer();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    std::vector<int> difid;
    std::vector<std::vector<char> > payload;
};

struct status
{
    status();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    int oraclestatus;
};

class statemachine
{
public:

    statemachine(yami::agent & client_agent,
        const std::string & server_location, const std::string & object_name,
        int timeout = 0);

    void initialise(status & res);
    void download(const config & conf);

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

    virtual void initialise(status & res) = 0;
    virtual void download(const config & conf) = 0;

    void operator()(yami::incoming_message & im_);
};

} // namespace odb

#endif // YAMI4_IDL_ODB_H_INCLUDED
