#include "Server.h"
#include <boost/bind/bind.hpp>
#include "Session.h"
#include <iostream>

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_context& context_, Marketplace& marketplace_, 
               DataBase& database_)
    : context(context_),
    acceptor(context_, tcp::endpoint(tcp::v4(), PORT_NUM)),
    market(marketplace_),
    database(database_)
{
    std::cout << "Server started! port:" << PORT_NUM << std::endl;

    std::shared_ptr<Session> newSession = 
        std::make_shared<Session>(context, market, database);

    acceptor.async_accept(newSession->socket(),
                          boost::bind(&Server::on_accept, this, newSession,
                                      boost::asio::placeholders::error));
}

void Server::on_accept(std::shared_ptr<Session> session,
    const boost::system::error_code& error)
{
    if (!error) {
        std::cout << "New connection!" << std::endl;

        session->start();

        std::shared_ptr<Session> newSession = 
            std::make_shared<Session>(context, market, database);

        acceptor.async_accept(newSession->socket(),
                              boost::bind(&Server::on_accept, this, newSession,
                                          boost::asio::placeholders::error));
    }
    else {
        std::cout << "Error occured: " << error << std::endl;
    }
}