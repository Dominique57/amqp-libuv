#include "uvwConnection.h"

namespace rbmq {

UvwConnection::UvwConnection(UvwConnectionHandler* handler, const AMQP::Login& login, const std::string& vhost)
    : AMQP::Connection(handler, login, vhost),
        _uvwConnection(handler),
        _buf{}
{
    // const auto temp = handler->getClient()->on<uvw::DataEvent>(
    _callbackHandler = handler->getClient()->on<uvw::DataEvent>(
        [this](const uvw::DataEvent& e, uvw::TCPHandle&) {
        std::cout << "CallbackHandler !\n";
        // Append data to the buffer
        _buf.insert(_buf.begin(), e.data.get(), e.data.get() + e.length);
        // Process buffer
        const auto bytesConsummed = this->parse(_buf.data(), _buf.size());
        // Remove processed bytes
        _buf.erase(_buf.begin(), _buf.begin() + bytesConsummed);
    });
}

UvwConnection::~UvwConnection()
{
    if (auto client = _uvwConnection->getClient())
        client->erase(_callbackHandler);
}

}
