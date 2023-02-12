#include "module.h"

namespace rbmq::example {

ModuleCommunicator::ModuleCommunicator()
    : _loop(uvw::Loop::getDefault()), // maybe dont use default loop
      _uvwConnHandler(*_loop, "127.0.0.1", 5672),
      _uvwConn(&_uvwConnHandler, AMQP::Login("guest","guest"), "/"),
      _channel(&_uvwConn),
      _threadHandle(),
      _asyncHandle(_loop->resource<uvw::AsyncHandle>()),
      _asyncTasks()
{
    _asyncHandle->on<uvw::ErrorEvent>([this](auto &&...) {
         std::cerr << "AsyncHandle error !\n";
         this->_loop->stop();
    });
    _asyncHandle->on<uvw::AsyncEvent>([this](const uvw::AsyncEvent &e, uvw::AsyncHandle &handle) {
        std::cerr << "AsyncHandle event !\n";
        while (const auto taskOpt = _asyncTasks.try_pop()) {
            taskOpt.value()();
        }
    });

    _threadHandle = std::thread(&rbmq::example::ModuleCommunicator::runThread, this);
}

ModuleCommunicator::~ModuleCommunicator() {
    _asyncTasks.push([this]() {
        this->_loop->stop();
    });
    _asyncHandle->send();
    if (_threadHandle.joinable())
        _threadHandle.join();
}

void ModuleCommunicator::publish(const std::string &exchange,
     const std::string &key, const std::string &message) {
    _asyncTasks.push([this, exchange, key, message]() {
        _channel.startTransaction();
        _channel.publish(exchange, key, message);
        _channel.commitTransaction();
    });
    _asyncHandle->send();
}

void ModuleCommunicator::registerMailbox(const std::string &exchange, const std::string &filter,
    const std::function<void(const AMQP::Message &, uint64_t, bool)> &callback) {
    _asyncTasks.push(
        [this, exchange, filter, f = std::move(callback)]
        () {
        _channel.declareQueue(AMQP::exclusive).onSuccess(
            [this, exchange = std::move(exchange), filter = std::move(filter), f = std::move(f)]
            (const std::string &name, uint32_t messagecount, uint32_t consumercount) {
            this->_channel.bindQueue(exchange, name, filter);
            // TODO: handle error
            this->_channel.consume(name)
                .onReceived(std::move(f));
            // TODO: handle error
        });
    });
    _asyncHandle->send();
}


bool ModuleCommunicator::syncRegisterMailbox(const std::string &exchange, const std::string &filter,
    const std::function<void(const AMQP::Message &, uint64_t, bool)> &f) {
    rbmq::utils::ConcurentQueue<bool> taskResult;
    _asyncTasks.push(
        [this, &taskResult, exchange, filter, f] () mutable {
        _channel.declareQueue(AMQP::exclusive)
        .onSuccess([this, &taskResult, exchange, filter, f = std::move(f)]
            (const std::string &name, uint32_t messagecount, uint32_t consumercount) mutable {
            this->_channel.bindQueue(exchange, name, filter)
            .onSuccess([this, &taskResult, name, f = std::move(f)]() mutable {
                this->_channel.consume(name)
                .onReceived(std::move(f))
                .onSuccess([this, &taskResult]() {
                    taskResult.push(true);
                }).onError([this, &taskResult](const char* message) {
                    std::cerr << "Consumme failed: `" << message << "` !\n";
                    taskResult.push(false);
                });
            }).onError([this, &taskResult](const char* message) {
                std::cerr << "BindQueue failed: `" << message << "` !\n";
                taskResult.push(false);
            });
        }).onError([this, &taskResult](const char* message) {
            std::cerr << "QueueDeclare failed: `" << message << "` !\n";
            taskResult.push(false);
        });
    });
    _asyncHandle->send();
    return taskResult.pop();
}

void ModuleCommunicator::runThread() {
    std::cout << " Thread started !\n";
    _loop->run();
}

}
