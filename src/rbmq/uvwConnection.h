#pragma once

#include <amqpcpp.h>
#include "uvwConnectionHandler.h"

namespace rbmq {

class UvwConnection : public AMQP::Connection {

public:
    UvwConnection(UvwConnectionHandler* handler, const AMQP::Login& login, const std::string& vhost)
        : AMQP::Connection(handler, login, vhost),
          _uvwConnection(handler),
          _buf{}
    {
        const auto temp = handler->getClient()->on<uvw::DataEvent>(
            [this](const uvw::DataEvent& e, uvw::TCPHandle&) {
            // Append data to the buffer
            _buf.insert(_buf.begin(), e.data.get(), e.data.get() + e.length);
            // Process buffer
            const auto bytesConsummed = this->parse(_buf.data(), _buf.size());
            // Remove processed bytes
            _buf.erase(_buf.begin(), _buf.begin() + bytesConsummed);
        });
        std::cout << typeid(temp).name() << std::endl;
    }

    UvwConnection(UvwConnectionHandler* handler, const AMQP::Login& login)
        : UvwConnection(handler, login, "/")
    {}

    UvwConnection(UvwConnectionHandler* handler, const std::string& vhost)
        : UvwConnection(handler, AMQP::Login(), vhost)
    {}

    UvwConnection(UvwConnectionHandler* handler)
        : UvwConnection(handler, AMQP::Login(), "/")
    {}

private:
    /// UvwConnection handler
    UvwConnectionHandler* _uvwConnection;

    /// Callback handler of the attached data reading function
    //uvw::TCPHandle::Connection _callbackHandler;

    /// Buffer containing last sent data not yet processed
    std::vector<char> _buf;
};

}