#pragma once

#include <thread>

#include <amqpcpp.h>
#include <rbmq/uvwConnectionHandler.h>
#include <rbmq/uvwConnection.h>
#include <rbmq/utils/concurentQueue.h>


namespace rbmq::example {

class RbmqCommunicator {

public:
    RbmqCommunicator(AMQP::Channel &channel);

    void publish(const std::string &exchange, const std::string &key,
        const std::string &message);

    void registerMailbox(std::string exchange, std::string filter,
        std::function<void(const AMQP::Message &, uint64_t, bool)> onData,
        std::function<void(void)> onSuccess,
        std::function<void(const char*)> onError);

private:
    /// amqp channel
    AMQP::Channel &_channel;

    /// Task queue for main loop
    rbmq::utils::ConcurentQueue<std::function<void(void)>> _asyncTasks;
};

}