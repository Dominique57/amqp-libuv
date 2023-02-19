#include "uvwConnection.h"

namespace rbmq {

UvwConnection::UvwConnection(UvwConnectionHandler* handler, const AMQP::Login& login, const std::string& vhost)
    : AMQP::Connection(handler, login, vhost),
        _uvwConnection(handler),
        _buf{}
{
    _buf.reserve(4096);
    // const auto temp = handler->getClient()->on<uvw::DataEvent>(
    _callbackHandler = handler->getClient()->on<uvw::DataEvent>(
        [this](const uvw::DataEvent& e, uvw::TCPHandle&) {
        // Ensure enough space allocation
        // auto oldSize = _buf.size();
        // auto newSize = oldSize + e.length;
        // if (_buf.size() < newSize)
            // _buf.resize(newSize);
        // memcpy(_buf.data() + oldSize, e.data.get(), e.length);

        _buf.insert(_buf.end(), e.data.get(), e.data.get() + e.length);
        const auto bytesConsummed = this->parse(_buf.data(), _buf.size());
        _buf.erase(_buf.begin(), _buf.begin() + bytesConsummed);
    });
}

UvwConnection::~UvwConnection()
{
    if (auto client = _uvwConnection->getClient())
        client->erase(_callbackHandler);
}

}
