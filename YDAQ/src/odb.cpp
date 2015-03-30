//
// C++ implementations for package ODB.
// This file was generated automatically by yami4idl.
//

#include "odb.h"

#include <yami4-cpp/agent.h>
#include <yami4-cpp/errors.h>
#include <yami4-cpp/incoming_message.h>
#include <yami4-cpp/outgoing_message.h>

using namespace Odb;

Config::Config()
{
}

void Config::write(yami::parameters & params) const
{
    params.set_string_shallow("dbstate",
        Dbstate.c_str(), Dbstate.size());
}

void Config::read(const yami::parameters & params)
{
    Dbstate = params.get_string("dbstate");
}

Dbbuffer::Dbbuffer()
{
}

void Dbbuffer::write(yami::parameters & params) const
{
    params.set_integer("difid", Difid);
    params.set_integer("nasic", Nasic);
    params.set_binary_shallow("payload",
        &Payload[0], Payload.size());
}

void Dbbuffer::read(const yami::parameters & params)
{
    Difid = params.get_integer("difid");
    Nasic = params.get_integer("nasic");
    {
        std::size_t size_;
        const char * buf_ = reinterpret_cast<const char *>(
            params.get_binary("payload", size_));
        Payload.assign(buf_, buf_ + size_);
    }
}

Status::Status()
{
    RunValid = false;
}

void Status::write(yami::parameters & params) const
{
    params.set_string_shallow("oraclestatus",
        Oraclestatus.c_str(), Oraclestatus.size());
    if (RunValid)
    {
    params.set_integer("run", Run);
    }
}

void Status::read(const yami::parameters & params)
{
    Oraclestatus = params.get_string("oraclestatus");
    yami::parameter_entry e_;
    RunValid = params.find("run", e_);
    if (RunValid)
    {
    Run = params.get_integer("run");
    }
}

Statemachine::Statemachine(yami::agent & client_agent,
    const std::string & server_location, const std::string & object_name,
    int timeout)
    : agent_(client_agent),
    server_location_(server_location),
    object_name_(object_name),
    timeout_(timeout)
{
}

void Statemachine::Initialise(Status & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "initialise"));

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
        Res.read(om_->get_reply());
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

void Statemachine::Download(const Config & Conf, Status & Res)
{
    yami::parameters Conf_;
    Conf.write(Conf_);
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "download", Conf_));

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
        Res.read(om_->get_reply());
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

void Statemachine::Dispatch(Status & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "dispatch"));

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
        Res.read(om_->get_reply());
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

void Statemachine::Newrun(Status & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "newrun"));

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
        Res.read(om_->get_reply());
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

void Statemachine::Currentrun(Status & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "currentrun"));

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
        Res.read(om_->get_reply());
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

void StatemachineServer::operator()(yami::incoming_message & im_)
{
    const std::string & msg_name_ = im_.get_message_name();

    if (msg_name_ == "initialise")
    {
        Status Res;

        Initialise(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "download")
    {
        Config Conf;
        Conf.read(im_.get_parameters());
        Status Res;

        Download(Conf, Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "dispatch")
    {
        Status Res;

        Dispatch(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "newrun")
    {
        Status Res;

        Newrun(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "currentrun")
    {
        Status Res;

        Currentrun(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    {
        throw yami::yami_runtime_error("Unknown operation name.");
    }
}

