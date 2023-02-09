#include "entrypoint.h"

#include <cassert>

#include <amqpcpp.h>
#include <uvw.hpp>
#include <uvw/emitter.h>
#include <uvw/handle.hpp>
#include <uvw/stream.h>
#include <uvw/tcp.h>
#include "rbmq/uvConnectionHandler.h"
#include "rbmq/uvwConnectionHandler.h"
#include "rbmq/uvwConnection.h"

namespace rbmq {

void bindTcpClient(const std::shared_ptr<uvw::TCPHandle>& handle) {
    using namespace uvw;

    handle->on<ConnectEvent>([](const ConnectEvent &, TCPHandle &){
        std::cout << "uvw::tcp connected !\n";
    });
    handle->on<WriteEvent>([](const WriteEvent &, TCPHandle &) {
        std::cout << "uvw::tcp write !\n";
    });
    handle->on<DataEvent>([](const DataEvent &, TCPHandle &) {
        std::cout << "uvw::tcp read data!\n";
    });
    handle->on<ErrorEvent>([](const ErrorEvent &e, TCPHandle &handle){
        std::cerr << "uvw::tcp error : `" << e.what() << "` !\n";
        handle.close();
    });
    handle->on<EndEvent>([](const EndEvent&, TCPHandle &) {
        std::cout << "uvw::tcp endevent !\n";
    });
    handle->on<CloseEvent>([](const CloseEvent &, TCPHandle &){
        std::cout << "uvw::tcp closed !\n";
    });
}

int entrypoint() {
    // Create libuv loop and tcp client socket
    std::shared_ptr<uvw::Loop> loop = uvw::Loop::getDefault(); // care its default loop
    assert(loop);
    std::shared_ptr<uvw::TCPHandle> client = loop->resource<uvw::TCPHandle>();
    assert(client);

    // Add callbacks to the clients for debug
    bindTcpClient(client);

    // Synchronous connection
    client->connect("127.0.0.1", 5672);
    loop->run();

    // Create amqp-cpp uvw connection handler
    UvwConnectionHandler uvwConHandler(client);
    UvwConnection connection(&uvwConHandler, AMQP::Login("guest","guest"), "/");
    //AMQP::Connection connection(&uvwConHandler, AMQP::Login("guest","guest"), "/");
    AMQP::Channel channel(&connection);
    channel.declareExchange("my-exchange", AMQP::fanout);
    channel.declareQueue("my-queue");
    channel.bindQueue("my-exchange", "my-queue", "my-routing-key");

    // Restart loop with tcp socket registered to keep reading until disconnection
    client->read();
    loop->run();

    // Finish program
    std::cout << "Finishing...\n";

    return 0;
}

int ignoreMe() {
    // create an instance of your own connection handler
    UvConnectionHandler myHandler;

    // create a AMQP connection object
    AMQP::Connection connection(&myHandler, AMQP::Login("guest","guest"), "/");

    // and create a channel
    AMQP::Channel channel(&connection);

    // use the channel object to call the AMQP method you like
    channel.declareExchange("my-exchange", AMQP::fanout);
    channel.declareQueue("my-queue");
    channel.bindQueue("my-exchange", "my-queue", "my-routing-key");

    return 0;
}

}
