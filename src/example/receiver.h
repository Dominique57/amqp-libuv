#pragma once

#include <thread>

#include <amqpcpp.h>
#include <rbmq/uvwConnectionHandler.h>
#include <rbmq/uvwConnection.h>
#include <rbmq/utils/concurentQueue.h>

namespace rbmq::example {

class ModuleReceiver {

public:
    ModuleReceiver();

    ~ModuleReceiver();

protected:
    void sendStopThread();

    void waitStopThread();

    void runThread();

public:
    bool syncRegisterMailbox(const std::string &exchange, const std::string &key,
        const std::function<void(const AMQP::Message &, uint64_t, bool)> &callback);

private:
    /// uvw loop handle
    std::shared_ptr<uvw::Loop> _loop;

    /// amqp-uvw connection handler
    rbmq::UvwConnectionHandler _uvwConnHandler;

    /// amqp-uvw connection
    rbmq::UvwConnection _uvwConn;

    /// amqp channel
    AMQP::Channel _channel;

    /// Thread handle
    std::thread _threadHandle;

    /// Async handle to add callbacks
    std::shared_ptr<uvw::AsyncHandle> _asyncHandle;

    /// Task queue for main loop
    rbmq::utils::ConcurentQueue<std::function<void(void)>> _asyncTasks;
};

}