//
// C++ implementations for package ZUPLV.
// This file was generated automatically by yami4idl.
//

#include "zuplv.h"

#include <yami4-cpp/agent.h>
#include <yami4-cpp/errors.h>
#include <yami4-cpp/incoming_message.h>
#include <yami4-cpp/outgoing_message.h>

using namespace Zuplv;

Config::Config()
{
}

void Config::write(yami::parameters & params) const
{
    params.set_string_shallow("serial",
        Serial.c_str(), Serial.size());
    params.set_integer("port", Port);
}

void Config::read(const yami::parameters & params)
{
    Serial = params.get_string("serial");
    Port = params.get_integer("port");
}

Status::Status()
{
}

void Status::write(yami::parameters & params) const
{
    params.set_string_shallow("zupstatus",
        Zupstatus.c_str(), Zupstatus.size());
    params.set_double_float("voltage", Voltage);
    params.set_double_float("voltageread", Voltageread);
    params.set_double_float("current", Current);
}

void Status::read(const yami::parameters & params)
{
    Zupstatus = params.get_string("zupstatus");
    Voltage = params.get_double_float("voltage");
    Voltageread = params.get_double_float("voltageread");
    Current = params.get_double_float("current");
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

void Statemachine::Check(Status & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "check"));

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

void Statemachine::On(Status & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "on"));

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

void Statemachine::Off(Status & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "off"));

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

void Statemachine::Initialise(const Config & Conf)
{
    yami::parameters Conf_;
    Conf.write(Conf_);
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "initialise", Conf_));

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

void Statemachine::Close()
{
    agent_.send_one_way(server_location_, object_name_, "close");
}

void StatemachineServer::operator()(yami::incoming_message & im_)
{
    const std::string & msg_name_ = im_.get_message_name();

    if (msg_name_ == "check")
    {
        Status Res;

        Check(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "on")
    {
        Status Res;

        On(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "off")
    {
        Status Res;

        Off(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "initialise")
    {
        Config Conf;
        Conf.read(im_.get_parameters());

        Initialise(Conf);

        im_.reply();
    }
    else
    if (msg_name_ == "close")
    {
        Close();
    }
    else
    {
        throw yami::yami_runtime_error("Unknown operation name.");
    }
}

