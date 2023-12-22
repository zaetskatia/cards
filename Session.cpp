#include "Session.h"
#include <iostream>

// Constructor
Session::Session(boost::asio::ip::tcp::socket socket, ServerLogic& serverLogic)
    : socket_(std::move(socket)), serverLogic_(serverLogic) {}

// Start the session by initiating the asynchronous read operation
void Session::start() {
    doRead();
}

// Asynchronously read data from the socket
void Session::doRead() {
    auto self = shared_from_this();
    boost::beast::http::async_read(socket_, buffer_, request_,
        [this, self](boost::beast::error_code ec, std::size_t) {
            try{
            if (!ec) {
                // Process the request
                processRequest();
            } else {
                std::cerr << "Read error: " << ec.message() << std::endl;
                // Handle read error (e.g., close the socket)
            }
            }
            catch(const std::exception& e) {
            std::cerr << "Exception in write handler: " << e.what() << std::endl;
        }
        });
}

// Process the request and initiate a response
void Session::processRequest() {
    auto response = serverLogic_.handleRequest(request_);

    // Initiate an asynchronous write operation to send the response
    doWrite(response);
}

// Asynchronously write the response to the socket
void Session::doWrite(std::shared_ptr<http::response<http::string_body>> response) {
    auto self = shared_from_this();
    if (!socket_.is_open()) 
    {
    std::cerr << "Socket is closed, cannot write." << std::endl;
    return;
    }

if (!response) 
{
    std::cerr << "Response object is null." << std::endl;
    return;
}
    boost::beast::http::async_write(socket_, *response,
        [this, self](boost::beast::error_code ec, std::size_t) {
            try
            {
                if (!ec) {
                // Write operation completed successfully
                // initiate another read operation here if you want to keep the connection open
                doRead();
            } else {
                std::cerr << "Write error: " << ec.message() << std::endl;
                // Handle write error (e.g., close the socket)
            }
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            
           
        });
}
