#pragma once

#include <amqpcpp.h>
#include <uvw/stream.h>
#include <uvw/tcp.h>
#include "uvwConnectionHandler.h"

namespace rbmq {

class UvwConnection : public AMQP::Connection {

public:
    UvwConnection(UvwConnectionHandler* handler, const AMQP::Login& login, const std::string& vhost);

    UvwConnection(UvwConnectionHandler* handler, const AMQP::Login& login)
        : UvwConnection(handler, login, "/")
    {}

    UvwConnection(UvwConnectionHandler* handler, const std::string& vhost)
        : UvwConnection(handler, AMQP::Login(), vhost)
    {}

    UvwConnection(UvwConnectionHandler* handler)
        : UvwConnection(handler, AMQP::Login(), "/")
    {}

    virtual ~UvwConnection();

private:
    /// UvwConnection handler
    UvwConnectionHandler* _uvwConnection;

    /// Callback handler of the attached data reading function
    uvw::TCPHandle::Connection<uvw::DataEvent> _callbackHandler;

    /// Buffer containing last sent data not yet processed
    std::vector<char> _buf;
};

}