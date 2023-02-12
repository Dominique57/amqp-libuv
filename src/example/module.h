#pragma once

#include <thread>

#include <amqpcpp.h>
#include <rbmq/uvwConnectionHandler.h>
#include <rbmq/uvwConnection.h>


namespace rbmq::example {

class ModuleCommunicator {

public:
    ModuleCommunicator();

    ~ModuleCommunicator();

    void publish(const std::string &exchange, const std::string &key,
        const std::string &message);

    void registerMailbox(const std::string &exchange, const std::string &key,
        const std::function<void(const AMQP::Message &, uint64_t, bool)> &callback);
        
    // addCallback


protected:

    void runThread();

private:
    /// uvw loop handle
    std::shared_ptr<uvw::Loop> _loop;

    /// TODO
    rbmq::UvwConnectionHandler _uvwConnHandler;

    /// TODO
    rbmq::UvwConnection _uvwConn;

    /// TODO
    AMQP::Channel _channel;

    /// Async handle to add callbacks
    std::shared_ptr<uvw::AsyncHandle> _asyncHandle;

    /// Thread handle
    std::thread _threadHandle;

};

}