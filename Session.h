#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "ServerLogic.h"

namespace beast = boost::beast;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::ip::tcp::socket socket, ServerLogic &serverLogic);
    void start();

private:
    boost::asio::ip::tcp::socket socket_;
    beast::flat_buffer buffer_;
    beast::http::request<beast::http::string_body> request_;
    std::shared_ptr<beast::http::response<beast::http::string_body>> response_ptr_;
    ServerLogic &serverLogic_;

    void doRead();
    void doWrite();
};