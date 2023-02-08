#include "entrypoint.h"

#include <amqpcpp.h>
#include "rbmq/uvConnectionHandler.h"

namespace rbmq {

int entrypoint() {
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
