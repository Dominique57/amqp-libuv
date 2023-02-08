#include "uvConnectionHandler.h"

namespace rbmq {

void UvConnectionHandler::onData(AMQP::Connection*, const char*, size_t)
{
    // @todo
    //  Add your own implementation, for example by doing a call to the
    //  send() system call. But be aware that the send() call may not
    //  send all data at once, so you also need to take care of buffering
    //  the bytes that could not immediately be sent, and try to send
    //  them again when the socket becomes writable again
    std::cout << "UvConnectionHandler: Data to be sent !\n";
}

void UvConnectionHandler::onReady(AMQP::Connection*)
{
    // @todo
    //  add your own implementation, for example by creating a channel
    //  instance, and start publishing or consuming
    std::cout << "UvConnectionHandler: On ready !\n";
}

void UvConnectionHandler::onError(AMQP::Connection*, const char *message)
{
    // @todo
    //  add your own implementation, for example by reporting the error
    //  to the user of your program, log the error, and destruct the
    //  connection object because it is no longer in a usable state
    std::cerr << "UvConnectionHandler: On error `" << message << "` !\n";
}

void UvConnectionHandler::onClosed(AMQP::Connection*)
{
    // @todo
    //  add your own implementation, for example by closing down the
    //  underlying TCP connection too
    std::cout << "UvConnectionHandler: On closed !\n";
}

}
