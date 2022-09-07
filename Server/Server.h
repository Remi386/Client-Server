#pragma once
#include <boost/asio.hpp>

class Marketplace;
class DataBase;
class Session;

class Server {
public:
    Server(boost::asio::io_context& context_, 
           Marketplace& marketplace_, 
           DataBase& database_);

    void on_accept(std::shared_ptr<Session> session,
                   const boost::system::error_code& error);

private:
    boost::asio::io_context& context;
    boost::asio::ip::tcp::acceptor acceptor;

    //Dependency injection
    Marketplace& market;
    DataBase& database;
};