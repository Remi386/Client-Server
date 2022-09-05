#include "Client.h"
#include <iostream>
#include <iomanip>
#include "../NetCommon/NetCommon.h"
#include "Secure.h"
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

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
            std::cout << "Possible commands: sign up, sign in, "
                         "trade, info, close, cancel"  << std::endl;
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
        else if (option == "cancel") {
            handleCancelRequest();
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
        }
        else {
            std::cout << "Unknown command: " << option << std::endl;
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

void Client::handleCancelRequest()
{
    if (clientID < 0) {
        std::cout << "You are not signed in!" << std::endl;
        return;
    }
    if (clientRequests.empty()) {
        std::cout << "You have no active trade requests."
                     " Please, fetch information with 'info' command"  << std::endl;
        return;
    }

    int index = -1;

    while (true) {
        std::cout << "Enter index of request to remove (count from 1): ";
        std::cin >> index;

        if (index - 1 >= 0 && index < clientRequests.size()) {
            break;
        }

        std::cout << "Wrong value!" << std::endl;
    }

    auto iter = clientRequests.begin();
    std::advance(iter, index);

    nlohmann::json request = iter->createJsonObject();;
    
    sendMessage(RequestType::CancelRequest, request);
}

void Client::handleGetInfoResponse(bool status, const nlohmann::json& message)
{
    clientRequests.clear();

    using local_adj = boost::date_time::c_local_adjustor<boost::posix_time::ptime>;
    using namespace boost::posix_time;

    if (!status) {
        std::cout << "Something went wrong: ";
        std::cout << message["Info"].get<std::string>() << std::endl;
    }
    else {
        auto& balance = message["Balance"];
            
        std::cout << "Your balance: " << balance[0] << " dollars and "
                    << balance[1] << " rubles.\n";

        auto& activeRequest = message["ActiveRequests"];

        if (activeRequest.empty())
        {
            std::cout << "\No active requests!\n";
        }
        else {
            std::cout << "\nYour active requests: \n";
            int counter = 0;

            for (auto& request : activeRequest) {

                //Unpacking properties
                std::string rTime = request["RegTime"];
                ptime regTime = from_iso_extended_string(rTime);

                TradeRequestType type = request["Type"];
                int64_t volume = request["Volume"];
                int64_t price = request["Price"];

                std::cout << "\t" << ++counter << ") " 
                          << (type == TradeRequestType::Buy ? "Buying " : "Selling ")
                          << volume << " dollars with "
                          << price << " rubles price. Published on "
                          << local_adj::utc_to_local(regTime) << std::endl;


                clientRequests.push_back(TradeRequest(clientID, volume, price, 
                                                      regTime, type));

            }
        }

        auto& tradeHistory = message["TradeHistory"];

        if (!tradeHistory.empty()) {
            std::cout << "\nYour trade history: \n";
            int counter = 0;

            for (auto& trade : tradeHistory) {
                std::string rTime = trade["RegTime"];
                ptime regTime = from_iso_extended_string(rTime);

                std::string cTime = trade["CloseTime"];
                ptime closeTime = from_iso_extended_string(cTime);
                    
                std::cout << "\t" << ++counter << ") " 
                          << (trade["Type"] == TradeRequestType::Buy ? "Bought " : "Sold ") 
                          << trade["Volume"] << " dollars with " << trade["Price"] 
                          <<  " rubles price. Published on " << local_adj::utc_to_local(regTime)
                          << ", Closed on " << local_adj::utc_to_local(closeTime)
                          << " PartnerID = " << trade["Partner"] << std::endl;
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

    request["TradeReqType"] = (temp == "buy") ? TradeRequestType::Buy : TradeRequestType::Sell;

    std::cout << "Enter trade volume: ";
    std::getline(std::cin, temp);
    int64_t volume = std::stol(temp);

    std::cout << "Enter trade price: ";
    std::getline(std::cin, temp);
    int64_t price = std::stol(temp);

    request["Volume"] = volume;
    request["Price"] = price;

    sendMessage(RequestType::CreateRequest, request);
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

    case ResponseType::RequestCompleted:
        std::cout << "Recieved notification: " 
                  << message["Info"].get<std::string>() << std::endl;
        break;

    case ResponseType::RequestResponse:
        std::cout << message["Info"].get<std::string>() << std::endl;
        break;

    case ResponseType::CancelInfo:
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

