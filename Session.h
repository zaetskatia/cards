#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "ServerLogic.h"

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::ip::tcp::socket socket, ServerLogic& serverLogic);
    void start();

private:
    boost::asio::ip::tcp::socket socket_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::request<boost::beast::http::string_body> request_;
    ServerLogic& serverLogic_;

    void doRead();
    void doWrite(std::shared_ptr<http::response<http::string_body>> response);
    void processRequest();



};

#endif // SESSION_H