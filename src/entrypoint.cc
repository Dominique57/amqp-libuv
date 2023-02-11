#include "entrypoint.h"

#include <cassert>

#include <amqpcpp.h>
#include <cstdint>
#include "rbmq/uvwConnectionHandler.h"
#include "rbmq/uvwConnection.h"

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
    channel.declareExchange("my-exchange", AMQP::fanout);
    channel.declareQueue("my-queue");
    channel.bindQueue("my-exchange", "my-queue", "my-routing-key");
    auto messageCb = [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
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

}
