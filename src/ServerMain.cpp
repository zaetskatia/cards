#include "ServerLogic.h"
#include "Session.h"
#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <thread>

namespace asio = boost::asio;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;

class Server
{
public:
    Server(asio::io_context &ioc, short port, ServerLogic &serverLogic)
        : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)), serverLogic_(serverLogic)
    {

        do_accept();
    }

private:
    asio::io_context &ioc_;
    tcp::acceptor acceptor_;
    ServerLogic &serverLogic_;
    void do_accept()
    {

        acceptor_.async_accept(
            [this](beast::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<Session>(std::move(socket), serverLogic_)->start();
                }
                do_accept();
            });
    }
};

int main()
{
    try
    {
        asio::io_context ioc;
        DatabaseAccess dbAccess("database.db");
        DataService dataService(dbAccess);
        ServerLogic serverLogic(dataService);
        Server server(ioc, 8080, serverLogic);

        ioc.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
