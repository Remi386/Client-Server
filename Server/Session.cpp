#include "Session.h"
#include <iostream>
#include <boost/bind.hpp>
#include "../NetCommon/TradeRequest.h"
#include "../NetCommon/NetCommon.h"
#include "Marketplace.h"
#include "DataBase.h"

constexpr int MAX_BUFFER_SIZE = 1024;

namespace asio = boost::asio;

void Session::start()
{
	readMessageTask();
}

Session::~Session()
{
	if (clientID >= 0) {
		std::cout << "Removing session from active, ID = " << clientID << std::endl;
		Marketplace::instance().removeSession(clientID);
	}
}

void Session::readMessageTask()
{
	asio::async_read_until(sock, buffer, "\n", 
						   boost::bind(&Session::readMessageHandler, shared_from_this(),
									   asio::placeholders::error,
									   asio::placeholders::bytes_transferred));
}

void Session::sendMessageTask()
{
	asio::async_write(sock, asio::buffer(reply),
					  boost::bind(&Session::sendMessageHandler, shared_from_this(),
								  asio::placeholders::error));
}

void Session::sendNotificationTask() 
{
	const std::string& messageRef = notificationBuffers.front();

	asio::async_write(sock, asio::buffer(messageRef.c_str(), messageRef.size()),
					  boost::bind(&Session::sendNotificationHandler,
								  shared_from_this(), asio::placeholders::error));
}

void Session::addSessionToActive()
{
	std::cout << "Adding session to active, ID=" << clientID << std::endl;
	Marketplace::instance().addSession(clientID, shared_from_this());
}

void Session::sendNotification(const std::string& content)
{
	//Prepare notification
	nlohmann::json response;
	nlohmann::json message;

	response["ResponseType"] = ResponseType::RequestCompleted;
	response["Status"] = true;
	message["Info"] = content;
	response["Message"] = message;

	//Add notification to buffer
	notificationBuffers.push_back(std::move(response.dump() + "\n"));

	if (notificationBuffers.size() <= 1) {
		//If size more than 1, then write sequence already started,
		//No need to start another
		sendNotificationTask();
	}
}

void Session::sendNotificationHandler(const boost::system::error_code& error)
{
	if (!error) {
		std::cout << "Sended notification, userID: " << clientID
			<< ", message: " << notificationBuffers.front() << std::endl;

		notificationBuffers.pop_front();

		//Repeat sending notifications until buffer not empty
		if (!notificationBuffers.empty()) {
			sendNotificationTask();
		}
	}
}

void Session::readMessageHandler(const boost::system::error_code& error,
						  size_t bytes)
{
	if (!error) {

		handleMessage();
		
		//discard processed bytes from buffer
		buffer.consume(bytes);

		//Register send task when we done reading
		sendMessageTask();
	}
}

void Session::sendMessageHandler(const boost::system::error_code& error)
{
	if (!error) {

		if(!reply.empty()) //If reply was empty, no message actually send
			std::cout << "Sended message: " << reply << std::endl;

		reply.clear(); //Message sended, so we can clear buffer

		//Register read task when we done sending
		readMessageTask();
	}
}

void Session::handleMessage()
{
	try {
		nlohmann::json request = nlohmann::json::parse(asio::buffers_begin(buffer.data()),
													   asio::buffers_end(buffer.data()));

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

		case RequestType::CreateRequest:
			handleTradeRequest(message);
			break;

		case RequestType::CancelRequest:
			handleCancelRequest(message);
			break;
		case RequestType::GetInfo:
			handleInfoRequest();
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

		nlohmann::json message;

		message["Info"] = "Some of json properties incorrect: " + std::string(e.what());
		createResponse(ResponseType::Error, false, std::move(message));
	}
}

void Session::handleSignUp(const nlohmann::json& clientMessage)
{
	bool status = false;
	nlohmann::json message;
	std::string login = clientMessage.at("Login");
	std::string password = clientMessage.at("Password");

	int64_t userID = DataBase::instance().registerNewUser(login, password);

	if (userID == -1) {
		status = false;
		message["Info"] = "This login already taken";

	}
	else { //success
		status = true;
		message["UserID"] = userID;
		clientID = userID;
		addSessionToActive();
	}

	createResponse(ResponseType::Registration, status, std::move(message));
}

void Session::handleSignIn(const nlohmann::json& clientMessage)
{
	bool status = false;
	nlohmann::json message;
	std::string login = clientMessage.at("Login");
	std::string password = clientMessage.at("Password");

	int64_t userID = DataBase::instance().getUserID(login, password);

	if (userID == -1) {
		status = false;
		message["Info"] = "This login doesn't exists";
	}
	else if (userID == -2) {
		status = false;
		message["Info"] = "Incorrect password";
	}
	else { //success
		status = true;
		message["UserID"] = userID;
		clientID = userID;
		addSessionToActive();
	}

	createResponse(ResponseType::Registration, status, std::move(message));
}

void Session::handleTradeRequest(const nlohmann::json& clientMessage)
{
	nlohmann::json message;

	int64_t volume = clientMessage.at("Volume");
	int64_t price = clientMessage.at("Price");
	TradeRequestType reqType = clientMessage.at("TradeReqType");

	if (volume <= 0 || price <= 0) {
		message["Info"] = "Some of arguments incorrect, request denied";
		createResponse(ResponseType::RequestResponse, false, std::move(message));
	}
	else {
		TradeRequest tradeRequest(clientID, volume, price, reqType);

		Marketplace::instance().handleTradeRequest(tradeRequest);

		message["Info"] = "Trade request successfully registered";
		createResponse(ResponseType::RequestResponse, true, std::move(message));
	}
}

void Session::handleInfoRequest()
{
	nlohmann::json message;
	nlohmann::json activeRequests = nlohmann::json::array();
	nlohmann::json tradeHistory = nlohmann::json::array();

	DataBase& db = DataBase::instance();
	Marketplace& market = Marketplace::instance();
	
	auto [dollars, rubles] = db.getClientInfo(clientID).getBalance();

	message["Balance"] = nlohmann::json::array();
	
	message["Balance"].push_back(dollars);
	message["Balance"].push_back(rubles);

	auto& activeRequestsList = market.getActiveRequests(clientID);

	if (!activeRequestsList.empty()) {
		for (auto& request : activeRequestsList) {
			activeRequests.push_back(request->createJsonObject());
		}
	}

	auto& requestsHistory = db.getClientTradeHistory(clientID);

	if (!requestsHistory.empty()) {
		for (auto& request : requestsHistory) {
			tradeHistory.push_back(request.createJsonObject());
		}
	}

	message["ActiveRequests"] = activeRequests;
	message["TradeHistory"] = tradeHistory;

	createResponse(ResponseType::ClientInfo, true, std::move(message));
}

void Session::handleCancelRequest(const nlohmann::json& message)
{
	nlohmann::json response;
	bool status = false;

	int64_t price = message["Price"];
	int64_t volume = message["Volume"];
	TradeRequestType tradeType = message["Type"];
	boost::posix_time::ptime regTime 
		= boost::posix_time::from_iso_extended_string(message["RegTime"].get<std::string>());

	TradeRequest reqToDelete(clientID, volume, price, regTime, tradeType);
	status = Marketplace::instance().cancelTradeRequest(reqToDelete);

	if (status) {
		response["Info"] = "Request successfully canceled";
	}
	else {
		response["Info"] = "Request not found (already completed or deleted)";
	}

	createResponse(ResponseType::CancelInfo, status, std::move(response));
}


void Session::createResponse(ResponseType type, bool status, nlohmann::json&& message)
{
	nlohmann::json response;

	response["ResponseType"] = type;
	response["Status"] = status;
	response.emplace("Message", std::move(message));

	reply = response.dump();
	reply.push_back('\n'); //delimeter for messages
}