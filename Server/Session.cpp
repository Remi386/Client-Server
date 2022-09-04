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

		RequestType reqType = request.at("RequestType");
		int64_t userID = request.at("UserID");
		nlohmann::json& message = request.at("Message");
		
		switch (reqType)
		{
		case RequestType::SignUp:
			handleSignUp(message);
			break;

		case RequestType::SignIn:
			handleSignIn(message);
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
		reply = "Some of json properties incorrect: " + std::string(e.what());
	}
}

void Session::handleSignUp(const nlohmann::json& clientMessage)
{
	nlohmann::json response;
	nlohmann::json message;
	std::string login = clientMessage.at("Login");
	std::string password = clientMessage.at("Password");

	response["ResponseType"] = ResponseType::Registration;
	
	int64_t userID = DataBase::instance().registerNewUser(login, password);

	if (userID == -1) {
		response["Status"] = false;
		message["Info"] = "This login already taken";

	}
	else {
		response["Status"] = true;
		message["UserID"] = userID;
	}

	response["Message"] = message;
	reply = response.dump();
}

void Session::handleSignIn(const nlohmann::json& clientMessage)
{
	nlohmann::json response;
	nlohmann::json message;
	std::string login = clientMessage.at("Login");
	std::string password = clientMessage.at("Password");

	response["ResponseType"] = ResponseType::Registration;

	int64_t userID = DataBase::instance().getUserID(login, password);

	if (userID == -1) {
		response["Status"] = false;
		message["Info"] = "This login doesn't exists";
	}
	else if (userID == -2) {
		response["Status"] = false;
		message["Info"] = "Incorrect password";
	}
	else {
		response["Status"] = true;
		message["UserID"] = userID;
	}

	response["Message"] = message;
	reply = response.dump();
}

void Session::handleTradeRequest(int64_t userID, RequestType type,
								 const nlohmann::json& clientMessage)
{
	nlohmann::json response;
	nlohmann::json message;

	int64_t volume = clientMessage.at("Volume");
	int64_t price = clientMessage.at("Price");
	TradeRequest::Type reqType = clientMessage.at("TradeReqType");

	if (volume <= 0 || price <= 0) {

		response["ResponseType"] = ResponseType::RequestResponse;
		response["Status"] = false;
		message["Info"] = "Some of arguments incorrect, request denied";
		response["Message"] = message;
		reply = response.dump();
	}
	else {
		TradeRequest tradeRequest(userID, volume, price, reqType);

		Marketplace::instance().handleTradeRequest(tradeRequest);

		response["ResponseType"] = ResponseType::RequestResponse;
		response["Status"] = true;
		message["Info"] = "Trade request successfully registered";
		response["Message"] = message;
		reply = response.dump();
	}
}

void Session::handleInfoRequest(int64_t userID)
{
	nlohmann::json response;
	nlohmann::json message;
	nlohmann::json activeRequests = nlohmann::json::array();
	nlohmann::json tradeHistory = nlohmann::json::array();

	DataBase& db = DataBase::instance();
	Marketplace& market = Marketplace::instance();
	
	auto [dollars, rubles] = db.getClientInfo(userID).getBalance();

	message["Balance"] = nlohmann::json::array();
	
	message["Balance"].push_back(dollars);
	message["Balance"].push_back(rubles);

	auto& activeRequestsList = market.getActiveRequests(userID);

	if (!activeRequestsList.empty()) {
		for (auto& request : activeRequestsList) {
			activeRequests.push_back(request->toString());
		}
	}

	auto& requestsHistory = db.getClientTradeHistory(userID);

	if (!requestsHistory.empty()) {
		for (auto& request : requestsHistory) {
			tradeHistory.push_back(request.toString());
		}
	}

	response["ResponseType"] = ResponseType::ClientInfo;
	response["Status"] = true;
	message["ActiveRequests"] = activeRequests;
	message["TradeHistory"] = tradeHistory;
	response["Message"] = message;
	reply = response.dump();
}