#pragma once

#include <amqpcpp.h>
#include <uvw.hpp>
#include <uvw/dns.h>
#include <uvw/tcp.h>

namespace rbmq {

class UvwConnectionHandler : public AMQP::ConnectionHandler
{
public:
    UvwConnectionHandler(uvw::Loop &loop, const std::string& ip, int port);

    virtual ~UvwConnectionHandler()
    {
        closeConnection();
    }

    void closeConnection();

    void addDebugEvent();

protected:
    /**
     *  Method that is called by the AMQP library every time it has data
     *  available that should be sent to RabbitMQ.
     *  @param  connection  pointer to the main connection object
     *  @param  data        memory buffer with the data that should be sent to RabbitMQ
     *  @param  size        size of the buffer
     */
    void onData(AMQP::Connection *connection, const char *data, size_t size) override;


    /**
     *  Method that is called by the AMQP library when the login attempt
     *  succeeded. After this method has been called, the connection is ready
     *  to use.
     *  @param  connection      The connection that can now be used
     */
    void onReady(AMQP::Connection *connection) override;

    /**
     *  Method that is called by the AMQP library when a fatal error occurs
     *  on the connection, for example because data received from RabbitMQ
     *  could not be recognized.
     *  @param  connection      The connection on which the error occurred
     *  @param  message         A human readable error message
     */
    void onError(AMQP::Connection *connection, const char *message) override;

    /**
     *  Method that is called when the connection was closed. This is the
     *  counter part of a call to Connection::close() and it confirms that the
     *  AMQP connection was correctly closed.
     *
     *  @param  connection      The connection that was closed and that is now unusable
     */
    void onClosed(AMQP::Connection *connection) override;

public:
    std::shared_ptr<uvw::TCPHandle> getClient() { return _client; }

protected:
    std::shared_ptr<uvw::TCPHandle> _client;

};

}
