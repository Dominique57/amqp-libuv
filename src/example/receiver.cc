#include "module.h"

namespace rbmq::example {

ModuleReceiver::ModuleReceiver()
    : _loop(uvw::Loop::create()),
      _uvwConnHandler(*_loop, "127.0.0.1", 5672),
      _uvwConn(&_uvwConnHandler, AMQP::Login("guest","guest"), "/"),
      _channel(&_uvwConn),
      _threadHandle(),
      _asyncHandle(_loop->resource<uvw::AsyncHandle>()),
      _asyncTasks()
{
    _asyncHandle->on<uvw::ErrorEvent>([this](auto &&...) {
         std::cerr << "Receiver AsyncHandle error !\n";
         this->_loop->stop();
    });
    _asyncHandle->on<uvw::AsyncEvent>([this](const uvw::AsyncEvent &e, uvw::AsyncHandle &handle) {
        std::cerr << "Receiver AsyncHandle event !\n";
        while (const auto taskOpt = _asyncTasks.try_pop()) {
            taskOpt.value()();
        }
    });
    // _channel.setQos(10, true);

    _threadHandle = std::thread(&rbmq::example::ModuleReceiver::runThread, this);
}

ModuleReceiver::~ModuleReceiver() {
    sendStopThread();
    waitStopThread();

    if (_asyncHandle) {
        _asyncHandle->close();
        _asyncHandle = nullptr;
    }

    if (_loop) {
        _loop->close();
        _loop = nullptr;
    }
}

void ModuleReceiver::sendStopThread() {
    _asyncTasks.push([this]() {
        this->_loop->stop();
    });
    _asyncHandle->send();
}

void ModuleReceiver::waitStopThread() {
    if (_threadHandle.joinable())
        _threadHandle.join();
}


void ModuleReceiver::runThread() {
    std::cout << "Receiver Thread started !\n";
    _loop->run();
}

bool ModuleReceiver::syncRegisterMailbox(const std::string &exchange, const std::string &filter,
    const std::function<void(const AMQP::Message &, uint64_t, bool)> &f) {
    rbmq::utils::ConcurentQueue<bool> taskResult;
    _asyncTasks.push(
        [this, &taskResult, exchange, filter, f] () mutable {
        _channel.declareQueue(AMQP::exclusive)
        .onSuccess([this, &taskResult, exchange, filter, f = std::move(f)]
            (const std::string &name, uint32_t messagecount, uint32_t consumercount) mutable {
            _channel.bindQueue(exchange, name, filter)
            .onSuccess([this, &taskResult, name, f = std::move(f)]() mutable {
                _channel.consume(name)
                .onReceived(std::move(f))
                .onSuccess([this, &taskResult]() {
                    std::cout << "Mailbox registered !\n";
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

}
