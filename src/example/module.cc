#include "module.h"

namespace rbmq::example {

ModuleCommunicator::ModuleCommunicator()
    : _sender(),
      _receiver()
{
}

void ModuleCommunicator::publish(const std::string &exchange,
     const std::string &key, const std::string &message) {
    _sender.publish(exchange, key, message);
}

bool ModuleCommunicator::syncRegisterMailbox(const std::string &exchange,
    const std::string &filter,
    const std::function<void(const AMQP::Message &, uint64_t, bool)> &f) {
    return _receiver.syncRegisterMailbox(exchange, filter, f);
}

}
