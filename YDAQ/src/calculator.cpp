//
// C++ implementations for package Calculator.
// This file was generated automatically by yami4idl.
//

#include "calculator.h"

#include <yami4-cpp/agent.h>
#include <yami4-cpp/errors.h>
#include <yami4-cpp/incoming_message.h>
#include <yami4-cpp/outgoing_message.h>

using namespace calculator;

operands::operands()
{
}

void operands::write(yami::parameters & params) const
{
    params.set_integer("a", a);
    params.set_integer("b", b);
}

void operands::read(const yami::parameters & params)
{
    a = params.get_integer("a");
    b = params.get_integer("b");
}

results::results()
{
    ratio_valid = false;
}

void results::write(yami::parameters & params) const
{
    params.set_integer("sum", sum);
    params.set_integer("difference", difference);
    params.set_integer("product", product);
    if (ratio_valid)
    {
    params.set_integer("ratio", ratio);
    }
}

void results::read(const yami::parameters & params)
{
    sum = params.get_integer("sum");
    difference = params.get_integer("difference");
    product = params.get_integer("product");
    yami::parameter_entry e_;
    ratio_valid = params.find("ratio", e_);
    if (ratio_valid)
    {
    ratio = params.get_integer("ratio");
    }
}

operations::operations(yami::agent & client_agent,
    const std::string & server_location, const std::string & object_name,
    int timeout)
    : agent_(client_agent),
    server_location_(server_location),
    object_name_(object_name),
    timeout_(timeout)
{
}

void operations::calculate(const operands & op, results & res)
{
    yami::parameters op_;
    op.write(op_);
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "calculate", op_));

    if (timeout_ != 0)
    {
        bool on_time_ = om_->wait_for_completion(timeout_);
        if (on_time_ == false)
        {
            throw yami::yami_runtime_error("Operation timed out.");
        }
    }
    else
    {
        om_->wait_for_completion();
    }

    const yami::message_state state_ = om_->get_state();
    switch (state_)
    {
    case yami::replied:
        res.read(om_->get_reply());
        break;
    case yami::abandoned:
        throw yami::yami_runtime_error(
            "Operation was abandoned due to communication errors.");
    case yami::rejected:
        throw yami::yami_runtime_error(
            "Operation was rejected: " + om_->get_exception_msg());

    // these are for completeness:
    case yami::posted:
    case yami::transmitted:
        break;
    }
}

void operations_server::operator()(yami::incoming_message & im_)
{
    const std::string & msg_name_ = im_.get_message_name();

    if (msg_name_ == "calculate")
    {
        operands op;
        op.read(im_.get_parameters());
        results res;

        calculate(op, res);

        yami::parameters res_;
        res.write(res_);
        im_.reply(res_);
    }
    else
    {
        throw yami::yami_runtime_error("Unknown operation name.");
    }
}

