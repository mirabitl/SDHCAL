//
// C++ type definitions for package Calculator.
// This file was generated automatically by yami4idl.
//

#ifndef YAMI4_IDL_CALCULATOR_H_INCLUDED
#define YAMI4_IDL_CALCULATOR_H_INCLUDED

#include <yami4-cpp/parameters.h>
#include <string>
#include <vector>

namespace yami
{
    class agent;
    class incoming_message;
}

namespace calculator
{

struct operands
{
    operands();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    int a;
    int b;
};

struct results
{
    results();

    void write(yami::parameters & params) const;
    void read(const yami::parameters & params);

    int sum;
    int difference;
    int product;
    bool ratio_valid;
    int ratio;
};

class operations
{
public:

    operations(yami::agent & client_agent,
        const std::string & server_location, const std::string & object_name,
        int timeout = 0);

    void calculate(const operands & op, results & res);

private:

    yami::agent & agent_;
    const std::string server_location_;
    const std::string object_name_;
    const std::size_t timeout_;
};

class operations_server
{
public:

    virtual ~operations_server() {}

    virtual void calculate(const operands & op, results & res) = 0;

    void operator()(yami::incoming_message & im_);
};

} // namespace calculator

#endif // YAMI4_IDL_CALCULATOR_H_INCLUDED
