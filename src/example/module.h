#pragma once

#include "sender.h"
#include "receiver.h"

namespace rbmq::example {

class ModuleCommunicator {

public:
    ModuleCommunicator();

    void publish(const std::string &exchange, const std::string &key,
        const std::string &message);

    bool syncRegisterMailbox(const std::string &exchange, const std::string &key,
        const std::function<void(const AMQP::Message &, uint64_t, bool)> &callback);

private:
    /// Sender
    ModuleSender _sender;

    /// Receiver
    ModuleReceiver _receiver;
};

}