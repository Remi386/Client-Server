#include "Client.h"
#include <iostream>
#include "../NetCommon/NetCommon.h"

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
    //nlohmann::json message = nlohmann::json::parse(begin, end);
    
    return std::string(begin, end);
}

void Client::sendMessage(RequestType type, const nlohmann::json& message)
{
    nlohmann::json request;
    request["RequestType"] = type;
    request["UserID"] = clientID;
    request["Message"] = message;
    std::string req = request.dump();
    boost::asio::write(sock, boost::asio::buffer(req, req.size()));
}

void Client::loop()
{
    auto to_lower = [](std::string& str) 
    {
        std::transform(str.begin(), str.end(), str.begin(), 
            [](char ch) {return std::tolower(ch); });
    };
    
    bool running = true;
    while (running && sock.is_open()) 
    {
        std::cout << "Enter option (type 'help' for help): ";

        std::string option;
        std::getline(std::cin, option);
        to_lower(option);

        if (option == "help") {
            std::cout << "Possible commands: sign up, trade, info, close" 
                      << std::endl;
        }
        else if (option == "sign up") {
            sendMessage(RequestType::SignUp, "");
            
            std::string response = readMessage();
            std::cout << "Got message: " << response << std::endl;
            
            nlohmann::json res = nlohmann::json::parse(response);
            int64_t userID = res.at("UserID");

            std::cout << "Your ID: " << userID << std::endl;
            clientID = userID;
        }
        else if (option == "trade") {

            if (clientID < 0) {
                std::cout << "You are not signed in!" << std::endl;
                continue;
            }

            nlohmann::json request;
            std::string temp;
            do {
                std::cout << "Enter type of operation, 'buy' or 'sell' ('exit'): ";
                std::getline(std::cin, temp);
                to_lower(option);
            } while (temp != "buy" && temp != "sell" && temp != "exit");
            
            if (temp == "exit")
                continue;

            /////////////////////////////Should remove/////////////////////////////////
            request["TradeReqType"] = (temp == "buy") ? 0 : 1;

            std::cout << "Enter trade volume: ";
            std::getline(std::cin, temp);
            int64_t volume = std::stoi(temp);
            
            std::cout << "Enter trade price: ";
            std::getline(std::cin, temp);
            int64_t price = std::stoi(temp);

            request["Volume"] = volume;
            request["Price"] = price;

            sendMessage(RequestType::Request, request);
        }
        else if (option == "info") {
            if (clientID < 0) {
                std::cout << "You are not signed in!" << std::endl;
                continue;
            }
            sendMessage(RequestType::GetInfo, "");
            std::cout << readMessage() << std::endl;

        }
        else if(option == "close")
        {
            std::cout << "Client stopped" << std::endl;
            running = false;
            continue;
        }
        else {
            std::cout << "Unknown command: " << option << std::endl;
            continue;
        }
    }
}