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
    params.set_integer_array_shallow("difid",
        &Difid[0], Difid.size());
    {
        std::size_t size_ = Payload.size();
        params.create_binary_array("payload", size_);
        for (std::size_t i_ = 0; i_ != size_; ++i_)
        {
            params.set_binary_in_array(
                "payload", i_, &Payload[i_][0], Payload[i_].size());
        }
    }
}

void Dbbuffer::read(const yami::parameters & params)
{
    {
        std::size_t size_;
        const int * buf_ = params.get_integer_array(
            "difid", size_);
        Difid.assign(buf_, buf_ + size_);
    }
    {
        std::size_t size_ = params.get_binary_array_length("payload");
        Payload.resize(size_);
        for (std::size_t i_ = 0; i_ != size_; ++i_)
        {
            std::size_t bufSize_;
            const char * buf_ = reinterpret_cast<const char *>(
                params.get_binary_in_array("payload", i_, bufSize_));
            Payload[i_].assign(buf_, buf_ + bufSize_);
        }
    }
}

Status::Status()
{
}

void Status::write(yami::parameters & params) const
{
    params.set_integer("oraclestatus", Oraclestatus);
}

void Status::read(const yami::parameters & params)
{
    Oraclestatus = params.get_integer("oraclestatus");
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

void Statemachine::Download(const Config & Conf)
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

        Download(Conf);

        im_.reply();
    }
    else
    {
        throw yami::yami_runtime_error("Unknown operation name.");
    }
}

