#include "entrypoint.h"

#include <cassert>

#include <amqpcpp.h>
#include <cstdint>
#include "rbmq/uvwConnectionHandler.h"
#include "rbmq/uvwConnection.h"
#include "example/module.h"

namespace rbmq {

int entrypoint() {
    // Create libuv loop and tcp client socket
    std::shared_ptr<uvw::Loop> loop = uvw::Loop::getDefault(); // care its default loop
    assert(loop);

    // Create amqp-cpp uvw connection handler
    UvwConnectionHandler uvwConHandler(*loop, "127.0.0.1", 5672);
    assert(uvwConHandler.getClient());
    uvwConHandler.addDebugEvent();

    UvwConnection connection(&uvwConHandler, AMQP::Login("guest","guest"), "/");
    AMQP::Channel channel(&connection);

    // Define all                          
    channel.declareExchange("my-exchange", AMQP::fanout);
    channel.declareQueue("my-queue");
    channel.bindQueue("my-exchange", "my-queue", "my-routing-key");
    auto messageCb = [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool) {
        const auto msg = std::string(message.body(), message.bodySize());
        std::cout << "message received: " << msg << std::endl;
        channel.ack(deliveryTag);
    };
    channel.consume("my-queue")
        .onReceived(messageCb);

    // Start loop with tcp socket registered to keep reading until disconnection
    loop->run();

    // Finish program
    std::cout << "Finishing...\n";

    return 0;
}

int exampleEntrypoint() {
    auto comm = example::ModuleCommunicator();
    auto onMsg = [](const AMQP::Message &msg, uint64_t, bool) {
        const auto str = std::string(msg.body(), msg.bodySize());
        std::cout << "Message received: `" << str << "` !\n";
    };
    comm.syncRegisterMailbox("toto", "toto", onMsg);

    for (auto i = 0U; i < 100000; ++i)
        comm.publish("toto", "toto", std::to_string(i) + " A message !");

    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}

}
