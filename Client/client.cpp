#include "Client.h"
#include <iostream>
#include <iomanip>
#include "../NetCommon/NetCommon.h"
#include "Secure.h"
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

//Helper function tolower case strings
void to_lower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(),
        [](char ch) {return std::tolower(ch); });
}

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

        //Register task
        boost::asio::async_read_until(sock, buffer, "\n",
            boost::bind(&Client::readMessage, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

        //run context in another thread
        clientThread = std::thread([this]() { context.run(); });

    }
    catch (std::exception& e) {
        std::cout << "Can not connect to server: " << e.what() << std::endl;
        return;
    }

    std::cout << "Connected to server, port:" << port << std::endl;
}

void Client::readMessage(const boost::system::error_code& error,
                                size_t bytes)
{
    if (!error) {

        auto begin = boost::asio::buffers_begin(buffer.data());
        auto end = boost::asio::buffers_end(buffer.data());

        nlohmann::json response = nlohmann::json::parse(begin, end);
        
        handleResponse(response);

        buffer.consume(bytes);

        boost::asio::async_read_until(sock, buffer, "\n",
            boost::bind(&Client::readMessage, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        
    }
}

void Client::sendMessage(RequestType type, const nlohmann::json& message)
{
    nlohmann::json request;
    
    request["RequestType"] = type;
    request["UserID"] = clientID;
    request["Message"] = message;
    
    std::string req = request.dump();
    req.push_back('\n'); //delimeter for messages

    boost::asio::write(sock, boost::asio::buffer(req, req.size()));
}

void Client::loop()
{
    bool running = true;
    while (running && sock.is_open()) 
    {
        std::cout << "Enter option (type 'help' for help): ";

        std::string option;
        std::getline(std::cin, option);
        to_lower(option);

        if (option == "help") {
            std::cout << "Possible commands: sign up, sign in, trade, info, close" 
                      << std::endl;
        }
        else if (option == "sign up") {
            handleRegistrationRequest(RequestType::SignUp);
        }
        else if (option == "sign in") {
            handleRegistrationRequest(RequestType::SignIn);
        }
        else if (option == "trade") {
            handleTradeRequest();
        }
        else if (option == "info") {
            if (clientID == -1) {
                std::cout << "You are not signed in!" << std::endl;
            }
            else {
                sendMessage(RequestType::GetInfo, "");
            }
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

void Client::handleRegistrationRequest(RequestType type)
{
    nlohmann::json message;
    std::string login, password;

    std::cout << "Enter login: ";
    std::getline(std::cin, login);

    std::cout << "Enter password: ";
    std::getline(std::cin, password);
    
    message["Login"] = login;
    message["Password"] = secure::hashPassword(password);

    sendMessage(type, message);
}

void Client::handleGetInfoResponse(bool status, const nlohmann::json& message)
{
        if (!status) {
            std::cout << "Something went wrong: ";
            std::cout << message["Info"].get<std::string>() << std::endl;
        }
        else {
            auto& balance = message["Balance"];
            
            std::cout << "Your balance: " << balance[0] << " dollars and "
                      << balance[1] << " rubles.\n";

            auto activeRequest = message["ActiveRequests"].get<std::list<std::string>>();

            if (!activeRequest.empty()) {
                std::cout << "\nYour active requests: \n";
                int counter = 0;

                for (auto& request : activeRequest) {
                    std::cout << "\t" << ++counter << ") " << request << std::endl;
                }
            }

            auto tradeHistory = message["TradeHistory"].get<std::list<std::string>>();

            if (!tradeHistory.empty()) {
                std::cout << "\nYour trade history: \n";
                int counter = 0;

                for (auto& trade : tradeHistory) {
                    std::cout << "\t" << ++counter << ") " << trade << std::endl;
                }
            }
        }
}

void Client::handleTradeRequest()
{
    if (clientID < 0) {
        std::cout << "You are not signed in!" << std::endl;
        return;
    }

    nlohmann::json request;
    std::string temp;

    do {
        std::cout << "Enter type of operation, 'buy' or 'sell' ('exit'): ";
        std::getline(std::cin, temp);
        to_lower(temp);

    } while (temp != "buy" && temp != "sell" && temp != "exit");

    if (temp == "exit")
        return;

    /////////////////////////////Should remove/////////////////////////////////
    request["TradeReqType"] = (temp == "buy") ? 0 : 1;

    std::cout << "Enter trade volume: ";
    std::getline(std::cin, temp);
    int64_t volume = std::stol(temp);

    std::cout << "Enter trade price: ";
    std::getline(std::cin, temp);
    int64_t price = std::stol(temp);

    request["Volume"] = volume;
    request["Price"] = price;

    sendMessage(RequestType::Request, request);
}

void Client::handleResponse(const nlohmann::json& response)
{
    ResponseType resType = response["ResponseType"];
    bool status = response["Status"];
    nlohmann::json message = response["Message"];

    std::cout << "\nResponse received: \n";

    switch (resType)
    {
    case ResponseType::Registration:
        if (status) {
            clientID = message["UserID"];
            std::cout << "Got ID: " << clientID << std::endl;
        }
        else {
            std::cout << "Registration failed: " 
                      << message["Info"].get<std::string>() << std::endl;
        }
        break;

    case ResponseType::Notification:
        std::cout << "Recieved notification: " 
                  << message["Info"].get<std::string>() << std::endl;
        break;

    case ResponseType::RequestResponse:
        std::cout << message["Info"].get<std::string>() << std::endl;
        break;

    case ResponseType::ClientInfo:
    {
        handleGetInfoResponse(status, message);
        break;
    }
    case ResponseType::Error:
        std::cout << "Error occured: " 
                  << message["Info"].get<std::string>() << std::endl;
        break;
    }
}

