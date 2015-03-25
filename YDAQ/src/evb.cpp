//
// C++ implementations for package EVB.
// This file was generated automatically by yami4idl.
//

#include "evb.h"

#include <yami4-cpp/agent.h>
#include <yami4-cpp/errors.h>
#include <yami4-cpp/incoming_message.h>
#include <yami4-cpp/outgoing_message.h>

using namespace Evb;

Config::Config()
{
}

void Config::write(yami::parameters & params) const
{
    params.set_string_shallow("shmpath",
        Shmpath.c_str(), Shmpath.size());
    params.set_integer("numberoffragment", Numberoffragment);
    params.set_string_shallow("monitoringpath",
        Monitoringpath.c_str(), Monitoringpath.size());
    params.set_integer("monitoringsize", Monitoringsize);
    params.set_string_shallow("outputmode",
        Outputmode.c_str(), Outputmode.size());
    params.set_string_shallow("outputpath",
        Outputpath.c_str(), Outputpath.size());
    params.set_integer("publishperiod", Publishperiod);
}

void Config::read(const yami::parameters & params)
{
    Shmpath = params.get_string("shmpath");
    Numberoffragment = params.get_integer("numberoffragment");
    Monitoringpath = params.get_string("monitoringpath");
    Monitoringsize = params.get_integer("monitoringsize");
    Outputmode = params.get_string("outputmode");
    Outputpath = params.get_string("outputpath");
    Publishperiod = params.get_integer("publishperiod");
}

Status::Status()
{
    RunValid = false;
    StarttimeValid = false;
    CompletedValid = false;
    EventValid = false;
}

void Status::write(yami::parameters & params) const
{
    params.set_string_shallow("evbstatus",
        Evbstatus.c_str(), Evbstatus.size());
    if (RunValid)
    {
    params.set_integer("run", Run);
    }
    if (StarttimeValid)
    {
    params.set_string_shallow("starttime",
        Starttime.c_str(), Starttime.size());
    }
    if (CompletedValid)
    {
    params.set_integer("completed", Completed);
    }
    if (EventValid)
    {
    params.set_integer("event", Event);
    }
}

void Status::read(const yami::parameters & params)
{
    Evbstatus = params.get_string("evbstatus");
    yami::parameter_entry e_;
    RunValid = params.find("run", e_);
    if (RunValid)
    {
    Run = params.get_integer("run");
    }
    StarttimeValid = params.find("starttime", e_);
    if (StarttimeValid)
    {
    Starttime = params.get_string("starttime");
    }
    CompletedValid = params.find("completed", e_);
    if (CompletedValid)
    {
    Completed = params.get_integer("completed");
    }
    EventValid = params.find("event", e_);
    if (EventValid)
    {
    Event = params.get_integer("event");
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

void Statemachine::Initialise(const Config & Conf, Status & Res)
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

void Statemachine::Start(Status & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "start"));

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

void Statemachine::Stop(Status & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "stop"));

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
        Config Conf;
        Conf.read(im_.get_parameters());
        Status Res;

        Initialise(Conf, Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "start")
    {
        Status Res;

        Start(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "stop")
    {
        Status Res;

        Stop(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    {
        throw yami::yami_runtime_error("Unknown operation name.");
    }
}

