#include "Client.h"
#include <iostream>
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;

Client::Client()
: context(), sock(context)
{
   
}

void Client::connect(const std::string& adress, const std::string& port)
{
    try {
        tcp::resolver resolver(context);
        tcp::resolver::query query(tcp::v4(), adress, port);
        tcp::resolver::iterator iterator = resolver.resolve(query);

        sock.connect(*iterator);
    }
    catch (std::exception& e) {
        std::cout << "Can not connect to server: " << e.what() << std::endl;
        return;
    }

    std::cout << "Connected to server, port:" << port << std::endl;
}

std::string Client::readMessage()
{
    boost::asio::streambuf buff;
    boost::asio::read_until(sock, buff, "\0");
    
    auto begin = boost::asio::buffers_begin(buff.data());
    auto end = boost::asio::buffers_end(buff.data());

    nlohmann::json message = nlohmann::json::parse(begin, end);
    
    return message["Message"];
}

void Client::sendMessage(const std::string& message)
{
    nlohmann::json request;
    request["RequestType"] = "Message";
    request["UserID"] = clientID;
    request["Message"] = message;
    std::string req = request.dump();
    boost::asio::write(sock, boost::asio::buffer(req, req.size()));
}
