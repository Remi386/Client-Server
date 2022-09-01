#include <boost/asio.hpp>
#include "../NetCommon/NetCommon.h"
#include <iostream>
#include <memory>
#include "Session.h"
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

class Server {
public:
	Server(boost::asio::io_context& context)
		: context_(context),
		  acceptor_(context, tcp::endpoint(tcp::v4(), PORT_NUM))
	{
        std::cout << "Server started! port:" << PORT_NUM << std::endl;

        std::shared_ptr<Session> newSession = std::make_shared<Session>(context_);

        acceptor_.async_accept(newSession->socket(), 
                               boost::bind(&Server::on_accept, this, newSession,
                                           boost::asio::placeholders::error));

	}

    void on_accept(std::shared_ptr<Session> session,
                   const boost::system::error_code& error)
    {
        if (!error) {
            std::cout << "New connection!" << std::endl;

            session->start();

            std::shared_ptr<Session> newSession = std::make_shared<Session>(context_);

            acceptor_.async_accept(newSession->socket(),
                                   boost::bind(&Server::on_accept, this, newSession,
                                               boost::asio::placeholders::error));
        }
        else {
            std::cout << "Error occured: " << error << std::endl;
        }
    }

private:
	boost::asio::io_context& context_;
	tcp::acceptor acceptor_;
};

int main()
{
    try
    {
        boost::asio::io_context context;

        Server s(context);

        context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}