#include "sender.h"

#include "timer.h"

namespace rbmq::example {

ModuleSender::ModuleSender()
    : _loop(uvw::Loop::create()),
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

    _threadHandle = std::thread(&rbmq::example::ModuleSender::runThread, this);
}

ModuleSender::~ModuleSender() {
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

void ModuleSender::sendStopThread() {
    _asyncTasks.push([this]() {
        this->_loop->stop();
    });
    _asyncHandle->send();
}

void ModuleSender::waitStopThread() {
    if (_threadHandle.joinable())
        _threadHandle.join();
}

void ModuleSender::runThread() {
    std::cout << " Thread started !\n";
    _loop->run();
}

void ModuleSender::publish(std::string exchange, std::string key,
    std::string message) {
    _asyncTasks.push(
        [this, exchange = std::move(exchange), key = std::move(key),
         message = std::move(message)] () mutable {
        _channel.publish(std::move(exchange), std::move(key), std::move(message));
    });
    _asyncHandle->send();
}

}
