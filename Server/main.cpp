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

//int main()
//{
//    {
//        auto& market = Marketplace::instance();
//        TradeRequest req1 = TradeRequest(0, 10, 62, TradeRequest::Type::Buy);
//        market.handleTradeRequest(req1);
//    }
//    fun();
//    {
//        auto& market = Marketplace::instance();
//        TradeRequest req2 = TradeRequest(1, 20, 63, TradeRequest::Type::Buy);
//        market.handleTradeRequest(req2);
//    }
//
//    {
//        auto& market = Marketplace::instance();
//        for (int i = 0; i < 3; ++i) {
//            auto& clientRequests = market.getActiveRequests(i);
//            
//            std::cout << "Active requests for client" << i << "\n";
//
//            if (clientRequests.empty()) {
//                std::cout << "No active requests\n";
//            }
//            else {
//                for (auto& request : clientRequests) {
//                    std::cout << request->toString() << std::endl;
//                }
//            }
//        }
//    }
//
//    {
//        auto& market = Marketplace::instance();
//        TradeRequest req3 = TradeRequest(2, 50, 61, TradeRequest::Type::Sell);
//        market.handleTradeRequest(req3);
//
//        for (int i = 0; i < 3; ++i) {
//            auto& clientRequests = market.getActiveRequests(i);
//
//            std::cout << "Active requests for client" << i << "\n";
//            if (clientRequests.empty()) {
//                std::cout << "No active requests\n";
//            }
//            else {
//                for (auto& request : clientRequests) {
//                    std::cout << request->toString() << std::endl;
//                }
//            }
//        }
//    }
//}