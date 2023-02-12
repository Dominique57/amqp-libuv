#include "communicator.h"

namespace rbmq::example {

RbmqCommunicator::RbmqCommunicator(AMQP::Channel &channel)
    : _channel(channel)
{}

void RbmqCommunicator::publish(const std::string &exchange,
     const std::string &key, const std::string &message) {
    _channel.startTransaction();
    _channel.publish(exchange, key, message);
    _channel.commitTransaction();
}


void RbmqCommunicator::registerMailbox(std::string exchange, std::string filter,
    std::function<void(const AMQP::Message &, uint64_t, bool)> onData,
    std::function<void(void)> onSuccess,
    std::function<void(const char*)> onError) {
    // TODO
    _channel.declareQueue(AMQP::exclusive)
    .onSuccess([this, exchange, filter, onData, onError, onSuccess]
        (const std::string &name, uint32_t messagecount, uint32_t consumercount) mutable {
        _channel.bindQueue(exchange, name, filter)
        .onSuccess([this, name, onData, onSuccess, onError]() {
            _channel.consume(name)
            .onReceived(onData)
            .onSuccess([this, onSuccess]() {
                onSuccess();
            }).onError([this, onError](const char* message) {
                std::cerr << "QueueDeclare failed: `" << message << "` !\n";
                onError(message);
            });
        }).onError([this, onError](const char* message) {
            std::cerr << "QueueDeclare failed: `" << message << "` !\n";
            onError(message);
        });
    }).onError([this, onError](const char* message) {
        std::cerr << "QueueDeclare failed: `" << message << "` !\n";
        onError(message);
    });
}

}
