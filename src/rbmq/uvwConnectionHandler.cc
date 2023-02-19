#include "uvwConnectionHandler.h"

namespace rbmq {

UvwConnectionHandler::UvwConnectionHandler(uvw::Loop &loop, const std::string& ip, int port)
    : _client(loop.resource<uvw::TCPHandle>())
{
    using namespace uvw;
    if (_client) {
        _client->on<ErrorEvent>([this](const ErrorEvent &, TCPHandle &){
            this->closeConnection();
        });
        _client->connect(ip, port);
        _client->read();
    }
}

void UvwConnectionHandler::closeConnection() {
    if (_client) {
        _client->clear();
        _client->close();
        _client = nullptr;
    }
}

void UvwConnectionHandler::addDebugEvent() {
    using namespace uvw;
    if (_client) {
        _client->on<ConnectEvent>([](const ConnectEvent &, TCPHandle &){
            std::cout << "uvw::tcp connected !\n";
        });
        _client->on<WriteEvent>([](const WriteEvent &, TCPHandle &) {
            std::cout << "uvw::tcp write !\n";
        });
        _client->on<DataEvent>([](const DataEvent &, TCPHandle &) {
            std::cout << "uvw::tcp read data!\n";
        });
        _client->on<ErrorEvent>([](const ErrorEvent &e, TCPHandle &){
            std::cerr << "uvw::tcp error : `" << e.what() << "` !\n";
        });
        _client->on<EndEvent>([](const EndEvent&, TCPHandle &) {
            std::cout << "uvw::tcp endevent !\n";
        });
        _client->on<CloseEvent>([](const CloseEvent &, TCPHandle &){
            std::cout << "uvw::tcp closed !\n";
        });
    }
}

void UvwConnectionHandler::onData(AMQP::Connection*, const char* data, size_t len)
{
    // @todo
    //  Add your own implementation, for example by doing a call to the
    //  send() system call. But be aware that the send() call may not
    //  send all data at once, so you also need to take care of buffering
    //  the bytes that could not immediately be sent, and try to send
    //  them again when the socket becomes writable again

    if (_client) {
        auto buffer = std::make_unique<char[]>(len);
        memcpy(buffer.get(), data, len);
        _client->write(std::move(buffer), len);
    }
}

void UvwConnectionHandler::onReady(AMQP::Connection*)
{
    // @todo
    //  add your own implementation, for example by creating a channel
    //  instance, and start publishing or consuming
    std::cout << "UvwConnectionHandler: On ready !\n";
}

void UvwConnectionHandler::onError(AMQP::Connection* con, const char *message)
{
    // @todo
    //  add your own implementation, for example by reporting the error
    //  to the user of your program, log the error, and destruct the
    //  connection object because it is no longer in a usable state
    std::cerr << "UvwConnectionHandler: On error `" << message << "` !\n";
    closeConnection();
}

void UvwConnectionHandler::onClosed(AMQP::Connection*)
{
    // @todo
    //  add your own implementation, for example by closing down the
    //  underlying TCP connection too
    std::cout << "UvwConnectionHandler: On closed !\n";
}

}
