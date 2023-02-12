#include "module.h"

namespace rbmq::example {

ModuleCommunicator::ModuleCommunicator()
    : _loop(uvw::Loop::getDefault()), // maybe dont use default loop
      _uvwConnHandler(*_loop, "127.0.0.1", 5672),
      _uvwConn(&_uvwConnHandler, AMQP::Login("guest","guest"), "/"),
      _channel(&_uvwConn),
      _asyncHandle(_loop->resource<uvw::AsyncHandle>())
{
    _asyncHandle->on<uvw::ErrorEvent>([this](auto &&...) {
         std::cerr << "AsyncHandle error !\n";
         this->_loop->stop();
    });
    _asyncHandle->on<uvw::AsyncEvent>([this](const uvw::AsyncEvent &e, uvw::AsyncHandle &handle) {
         std::cerr << "AsyncHandle event: stopping loop !\n";
         this->_loop->stop();
    });

    _threadHandle = std::thread(&rbmq::example::ModuleCommunicator::runThread, this);
}

ModuleCommunicator::~ModuleCommunicator() {
    _asyncHandle->send();
    if (_threadHandle.joinable())
        _threadHandle.join();
}

void ModuleCommunicator::publish(const std::string &exchange,
     const std::string &key, const std::string &message) {
    // TODO: Thread data rade might occur, need to use asyncHandle !
    _channel.startTransaction();
    _channel.publish(exchange, key, message);
    _channel.commitTransaction();
}

void ModuleCommunicator::registerMailbox(const std::string &exchange, const std::string &filter,
    const std::function<void(const AMQP::Message &, uint64_t, bool)> &callback) {
    // TODO: Thread data rade might occur, need to use asyncHandle !
    _channel.declareQueue(AMQP::exclusive).onSuccess([this, exchange, filter, f = std::move(callback)]
        (const std::string &name, uint32_t messagecount, uint32_t consumercount) {
        this->_channel.bindQueue(exchange, name, filter);
        // TODO: handle error
        this->_channel.consume(name)
            .onReceived(std::move(f));
        // TODO: handle error
    });
}


void ModuleCommunicator::runThread() {
    std::cout << " Thread started !\n";
    _loop->run();
}

}
