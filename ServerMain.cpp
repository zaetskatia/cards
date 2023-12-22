#include "ServerLogic.h"
#include "Session.h"
#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <thread>

using boost::asio::ip::tcp;

int main() {
    try {
        // Initialize Boost.Asio IO context
        boost::asio::io_context io_context;

        // Create a work guard to prevent io_context from running out of work
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard(io_context.get_executor());

        // Set up server socket to accept connections on TCP port 8080
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

        // Database and logic setup
        DatabaseAccess dbAccess("database.db");
        DataService dataService(dbAccess);
        ServerLogic serverLogic(dataService);

        // Function to asynchronously accept connections
        std::function<void()> doAccept;
        doAccept = [&]() {
            acceptor.async_accept(
                [&io_context, &doAccept, &serverLogic](boost::beast::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        // Create a new Session for each connection and start it
                        std::make_shared<Session>(std::move(socket), serverLogic)->start();
                    }
                    doAccept(); // Continue accepting connections
                });
        };

        doAccept(); // Start accepting connections

        // Run io_context in a pool of threads
        std::vector<std::thread> thread_pool;
        for (std::size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
            thread_pool.emplace_back([&io_context]() {
                io_context.run();
            });
        }

        // Join all threads
        for (auto& thread : thread_pool) {
            thread.join();
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
