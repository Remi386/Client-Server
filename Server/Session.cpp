#include "Session.h"
#include <iostream>
#include <boost/bind.hpp>
#include "TradeRequest.h"
#include "../NetCommon/NetCommon.h"
#include "Marketplace.h"
#include "DataBase.h"

namespace asio = boost::asio;

void Session::start()
{
	sock.async_receive(asio::buffer(buffer, MAX_BUFFER_SIZE),
						 boost::bind(&Session::readMessage, shared_from_this(),
									 asio::placeholders::error,
									 asio::placeholders::bytes_transferred));
}

void Session::readMessage(const boost::system::error_code& error,
						  size_t bytes)
{
	if (!error) {
		buffer[bytes] = '\0';

		handleMessage();
		
		boost::asio::async_write(sock, asio::buffer(reply),
								 boost::bind(&Session::sendMessage, shared_from_this(),
											 asio::placeholders::error));
	}
}

void Session::sendMessage(const boost::system::error_code& error)
{
	if (!error) {
		if(!reply.empty()) //If reply was empty, no message actually send
			std::cout << "Sended message: " << reply << std::endl;

		reply.clear(); //Message sended, so we can clear buffer

		sock.async_receive(asio::buffer(buffer, MAX_BUFFER_SIZE),
							boost::bind(&Session::readMessage, shared_from_this(),
										asio::placeholders::error,
										asio::placeholders::bytes_transferred));
	}
}

void Session::handleMessage()
{
	try {
		nlohmann::json request = nlohmann::json::parse(buffer);

		std::cout << "Got message: " << request << std::endl;

		auto reqType = request.at("RequestType").get<RequestType>();
		auto userID = request.at("UserID").get<int64_t>();
		auto message = request.at("Message");
		
		switch (reqType)
		{
		case RequestType::SignUp:
			handleSignUp();
			break;

		case RequestType::SignIn:
			handleSignIn(userID, message);
			break;

		case RequestType::Request:
			handleTradeRequest(userID, reqType, message);
			break;

		case RequestType::GetInfo:
			handleInfoRequest(userID);
			break;

		case RequestType::Close: //Close logic???
			break;

		default:
			reply = "Unknown request type: " + std::to_string(uint32_t(reqType));
			break;
		}

	}
	catch (const std::exception& e) {
		std::cout << "Exeption occured while parsing json: " << e.what() << std::endl;
	}
}

void Session::handleSignUp()
{
	nlohmann::json response;
	response["UserID"] = DataBase::instance().registerNewUser();
	reply = response.dump();
}

void Session::handleSignIn(int64_t userID, const nlohmann::json& message)
{
	//Some password logic
}

void Session::handleTradeRequest(int64_t userID, RequestType type,
								 const nlohmann::json& message)
{
	try {
		auto volume = message.at("Volume").get<int64_t>();
		auto price = message.at("Price").get<int64_t>();
		auto reqType = message.at("TradeReqType").get<TradeRequest::Type>();
		TradeRequest tradeRequest(userID, volume, price, reqType);
		Marketplace::instance().handleTradeRequest(tradeRequest);
	}
	catch (const std::exception& e) {
		reply = "Some of json properties missing: " + std::string(e.what());
	}
}

void Session::handleInfoRequest(int64_t userID)
{
	DataBase& db = DataBase::instance();
	Marketplace& market = Marketplace::instance();

	auto [dollars, rubles] = db.getClientInfo(userID).getBalance();
	reply = "Your ID = " + std::to_string(userID) + ", balance = "
		    + std::to_string(dollars) + " dollars, "
		    + std::to_string(rubles) + " rubles.\n";

	auto& activeRequests = market.getActiveRequests(userID);

	if (!activeRequests.empty()) {
		reply += "Active trade requests: ";
		for (auto& request : activeRequests) {
			reply += request->toString();
			reply.push_back('\n');
		}	
	}

	auto& requestsHistory = db.getClientTradeHistory(userID);

	if (!requestsHistory.empty()) {
		reply += "Your trade history: ";
		for (auto& complitedRequest : requestsHistory) {
			reply += complitedRequest.toString();
			reply.push_back('\n');
		}
	}
}