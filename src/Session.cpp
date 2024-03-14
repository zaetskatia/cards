#include "Session.h"
#include <iostream>

Session::Session(boost::asio::ip::tcp::socket socket, ServerLogic &serverLogic)
    : socket_(std::move(socket)), serverLogic_(serverLogic) {}

void Session::start()
{
    doRead();
}

void Session::doRead()
{
    request_ = {};
    auto self = shared_from_this();
    buffer_.consume(buffer_.size());
    boost::beast::http::async_read(socket_, buffer_, request_,
                                   [this, self](boost::beast::error_code ec, std::size_t)
                                   {
                                       try
                                       {
                                           if (!ec)
                                           {
                                               response_ptr_ = serverLogic_.handleRequest(request_);
                                               doWrite();
                                           }
                                           else
                                           {
                                               std::cerr << "Read error: " << ec.message() << std::endl;
                                               // Handle read error (close the socket?)
                                           }
                                       }
                                       catch (const std::exception &e)
                                       {
                                           std::cerr << "Exception in write handler: " << e.what() << std::endl;
                                       }
                                   });
}

void Session::doWrite()
{
    auto self(shared_from_this());
    beast::http::async_write(socket_, *response_ptr_,
                             [this, self](beast::error_code ec, std::size_t)
                             {
                                 if (!ec)
                                 {
                                     doRead();
                                 }
                                 else
                                 {
                                     std::cerr << "Write error: " << ec.message() << std::endl;
                                     socket_.shutdown(tcp::socket::shutdown_send, ec);
                                 }
                             });
}
