#include "Client.h"
#include <iostream>

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
    std::string message(std::istreambuf_iterator<char>(&buff), {});

    return message;
}

void Client::sendMessage(const std::string& message)
{
    boost::asio::write(sock, boost::asio::buffer(message, message.size()));
}
