//
// C++ implementations for package EVB.
// This file was generated automatically by yami4idl.
//

#include "evb.h"

#include <yami4-cpp/agent.h>
#include <yami4-cpp/errors.h>
#include <yami4-cpp/incoming_message.h>
#include <yami4-cpp/outgoing_message.h>

using namespace evb;

config::config()
{
}

void config::write(yami::parameters & params) const
{
    params.set_string_shallow("shmpath",
        shmpath.c_str(), shmpath.size());
    params.set_integer("numberoffragment", numberoffragment);
    params.set_string_shallow("monitoringpath",
        monitoringpath.c_str(), monitoringpath.size());
    params.set_integer("monitoringsize", monitoringsize);
    params.set_string_shallow("outputmode",
        outputmode.c_str(), outputmode.size());
    params.set_string_shallow("outputpath",
        outputpath.c_str(), outputpath.size());
    params.set_integer("publishperiod", publishperiod);
}

void config::read(const yami::parameters & params)
{
    shmpath = params.get_string("shmpath");
    numberoffragment = params.get_integer("numberoffragment");
    monitoringpath = params.get_string("monitoringpath");
    monitoringsize = params.get_integer("monitoringsize");
    outputmode = params.get_string("outputmode");
    outputpath = params.get_string("outputpath");
    publishperiod = params.get_integer("publishperiod");
}

status::status()
{
    run_valid = false;
    starttime_valid = false;
    completed_valid = false;
    event_valid = false;
}

void status::write(yami::parameters & params) const
{
    params.set_string_shallow("evbstatus",
        evbstatus.c_str(), evbstatus.size());
    if (run_valid)
    {
    params.set_integer("run", run);
    }
    if (starttime_valid)
    {
    params.set_string_shallow("starttime",
        starttime.c_str(), starttime.size());
    }
    if (completed_valid)
    {
    params.set_integer("completed", completed);
    }
    if (event_valid)
    {
    params.set_integer("event", event);
    }
}

void status::read(const yami::parameters & params)
{
    evbstatus = params.get_string("evbstatus");
    yami::parameter_entry e_;
    run_valid = params.find("run", e_);
    if (run_valid)
    {
    run = params.get_integer("run");
    }
    starttime_valid = params.find("starttime", e_);
    if (starttime_valid)
    {
    starttime = params.get_string("starttime");
    }
    completed_valid = params.find("completed", e_);
    if (completed_valid)
    {
    completed = params.get_integer("completed");
    }
    event_valid = params.find("event", e_);
    if (event_valid)
    {
    event = params.get_integer("event");
    }
}

statemachine::statemachine(yami::agent & client_agent,
    const std::string & server_location, const std::string & object_name,
    int timeout)
    : agent_(client_agent),
    server_location_(server_location),
    object_name_(object_name),
    timeout_(timeout)
{
}

void statemachine::initialise(const config & conf, status & res)
{
    yami::parameters conf_;
    conf.write(conf_);
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "initialise", conf_));

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

void statemachine::start(status & res)
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

void statemachine::stop(status & res)
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

void statemachine_server::operator()(yami::incoming_message & im_)
{
    const std::string & msg_name_ = im_.get_message_name();

    if (msg_name_ == "initialise")
    {
        config conf;
        conf.read(im_.get_parameters());
        status res;

        initialise(conf, res);

        yami::parameters res_;
        res.write(res_);
        im_.reply(res_);
    }
    else
    if (msg_name_ == "start")
    {
        status res;

        start(res);

        yami::parameters res_;
        res.write(res_);
        im_.reply(res_);
    }
    else
    if (msg_name_ == "stop")
    {
        status res;

        stop(res);

        yami::parameters res_;
        res.write(res_);
        im_.reply(res_);
    }
    else
    {
        throw yami::yami_runtime_error("Unknown operation name.");
    }
}

