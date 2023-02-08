#include "uvwConnectionHandler.h"

namespace rbmq {

void UvwConnectionHandler::onData(AMQP::Connection*, const char*, size_t)
{
    // @todo
    //  Add your own implementation, for example by doing a call to the
    //  send() system call. But be aware that the send() call may not
    //  send all data at once, so you also need to take care of buffering
    //  the bytes that could not immediately be sent, and try to send
    //  them again when the socket becomes writable again
    std::cout << "UvwConnectionHandler: Data to be sent !\n";
}

void UvwConnectionHandler::onReady(AMQP::Connection*)
{
    // @todo
    //  add your own implementation, for example by creating a channel
    //  instance, and start publishing or consuming
    std::cout << "UvwConnectionHandler: On ready !\n";
}

void UvwConnectionHandler::onError(AMQP::Connection*, const char *message)
{
    // @todo
    //  add your own implementation, for example by reporting the error
    //  to the user of your program, log the error, and destruct the
    //  connection object because it is no longer in a usable state
    std::cerr << "UvwConnectionHandler: On error `" << message << "` !\n";
}

void UvwConnectionHandler::onClosed(AMQP::Connection*)
{
    // @todo
    //  add your own implementation, for example by closing down the
    //  underlying TCP connection too
    std::cout << "UvwConnectionHandler: On closed !\n";
}

}
