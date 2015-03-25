//
// C++ implementations for package DIF.
// This file was generated automatically by yami4idl.
//

#include "dif.h"

#include <yami4-cpp/agent.h>
#include <yami4-cpp/errors.h>
#include <yami4-cpp/incoming_message.h>
#include <yami4-cpp/outgoing_message.h>

using namespace Dif;

Config::Config()
{
}

void Config::write(yami::parameters & params) const
{
    params.set_integer("trigger", Trigger);
    params.set_string_shallow("dbstate",
        Dbstate.c_str(), Dbstate.size());
}

void Config::read(const yami::parameters & params)
{
    Trigger = params.get_integer("trigger");
    Dbstate = params.get_string("dbstate");
}

Difstatus::Difstatus()
{
    GtcValid = false;
    BcidValid = false;
}

void Difstatus::write(yami::parameters & params) const
{
    params.set_integer_array_shallow("status",
        &Status[0], Status.size());
    {
        std::size_t size_ = Debug.size();
        params.create_string_array("debug", size_);
        for (std::size_t i_ = 0; i_ != size_; ++i_)
        {
            params.set_string_in_array("debug", i_, Debug[i_]);
        }
    }
    if (GtcValid)
    {
    params.set_integer_array_shallow("gtc",
        &Gtc[0], Gtc.size());
    }
    if (BcidValid)
    {
    params.set_integer_array_shallow("bcid",
        &Bcid[0], Bcid.size());
    }
}

void Difstatus::read(const yami::parameters & params)
{
    {
        std::size_t size_;
        const int * buf_ = params.get_integer_array(
            "status", size_);
        Status.assign(buf_, buf_ + size_);
    }
    {
        std::size_t size_ = params.get_string_array_length("debug");
        Debug.resize(size_);
        for (std::size_t i_ = 0; i_ != size_; ++i_)
        {
            Debug[i_] = params.get_string_in_array("debug", i_);
        }
    }
    yami::parameter_entry e_;
    GtcValid = params.find("gtc", e_);
    if (GtcValid)
    {
    {
        std::size_t size_;
        const int * buf_ = params.get_integer_array(
            "gtc", size_);
        Gtc.assign(buf_, buf_ + size_);
    }
    }
    BcidValid = params.find("bcid", e_);
    if (BcidValid)
    {
    {
        std::size_t size_;
        const int * buf_ = params.get_integer_array(
            "bcid", size_);
        Bcid.assign(buf_, buf_ + size_);
    }
    }
}

Scanstatus::Scanstatus()
{
}

void Scanstatus::write(yami::parameters & params) const
{
    params.set_integer_array_shallow("diflist",
        &Diflist[0], Diflist.size());
}

void Scanstatus::read(const yami::parameters & params)
{
    {
        std::size_t size_;
        const int * buf_ = params.get_integer_array(
            "diflist", size_);
        Diflist.assign(buf_, buf_ + size_);
    }
}

Data::Data()
{
}

void Data::write(yami::parameters & params) const
{
    params.set_integer("difid", Difid);
    params.set_integer("gtc", Gtc);
    params.set_integer("bcid", Bcid);
    params.set_binary_shallow("payload",
        &Payload[0], Payload.size());
}

void Data::read(const yami::parameters & params)
{
    Difid = params.get_integer("difid");
    Gtc = params.get_integer("gtc");
    Bcid = params.get_integer("bcid");
    {
        std::size_t size_;
        const char * buf_ = reinterpret_cast<const char *>(
            params.get_binary("payload", size_));
        Payload.assign(buf_, buf_ + size_);
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

void Statemachine::Scan(Scanstatus & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "scan"));

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

void Statemachine::Configure(const Config & Conf, Difstatus & Res)
{
    yami::parameters Conf_;
    Conf.write(Conf_);
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "configure", Conf_));

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

void Statemachine::Initialise(const Scanstatus & Conf, Difstatus & Res)
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

void Statemachine::Start(Difstatus & Res)
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

void Statemachine::Stop(Difstatus & Res)
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

void Statemachine::Destroy(Difstatus & Res)
{
    std::auto_ptr<yami::outgoing_message> om_(
        agent_.send(server_location_, object_name_, "destroy"));

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

void Statemachine::Processsc(const Odb::Dbbuffer & Buf)
{
    yami::parameters Buf_;
    Buf.write(Buf_);
    agent_.send_one_way(server_location_, object_name_, "processsc", Buf_);
}

void StatemachineServer::operator()(yami::incoming_message & im_)
{
    const std::string & msg_name_ = im_.get_message_name();

    if (msg_name_ == "scan")
    {
        Scanstatus Res;

        Scan(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "configure")
    {
        Config Conf;
        Conf.read(im_.get_parameters());
        Difstatus Res;

        Configure(Conf, Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "initialise")
    {
        Scanstatus Conf;
        Conf.read(im_.get_parameters());
        Difstatus Res;

        Initialise(Conf, Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "start")
    {
        Difstatus Res;

        Start(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "stop")
    {
        Difstatus Res;

        Stop(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "destroy")
    {
        Difstatus Res;

        Destroy(Res);

        yami::parameters Res_;
        Res.write(Res_);
        im_.reply(Res_);
    }
    else
    if (msg_name_ == "processsc" || msg_name_=="subscription_update" )
    {
        Odb::Dbbuffer Buf;
        Buf.read(im_.get_parameters());

        Processsc(Buf);
    }
    else
    {
        throw yami::yami_runtime_error("Unknown operation name.");
    }
}

