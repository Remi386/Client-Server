#include "Session.h"
#include <iostream>
#include <boost/bind/bind.hpp>
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
		market.removeSession(clientID);
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
	market.addSession(clientID, shared_from_this());
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
		//Read json from buffer
		nlohmann::json request = nlohmann::json::parse(asio::buffers_begin(buffer.data()),
												       asio::buffers_end(buffer.data()));

		//Process it and get response object
		nlohmann::json response = handleRequest(std::move(request));
		
		//Create reply buffer
		reply = response.dump();
		reply.push_back('\n'); //delimeter for messages

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

nlohmann::json Session::handleRequest(nlohmann::json&& request)
{
	nlohmann::json response;

	try {
		
		std::cout << "Got message: " << request << std::endl;

		RequestType reqType = request.at("RequestType");
		int64_t userID = request.at("UserID");
		nlohmann::json& message = request.at("Message");

		switch (reqType)
		{
		case RequestType::SignUp:
			response = handleSignUp(message);
			break;

		case RequestType::SignIn:
			response = handleSignIn(message);
			break;

		case RequestType::CreateRequest:
			response = handleTradeRequest(message);
			break;

		case RequestType::CancelRequest:
			response = handleCancelRequest(message);
			break;
		case RequestType::GetInfo:
			response = handleInfoRequest();
			break;

		case RequestType::Close: //Close logic???
			break;

		default:
		{
			nlohmann::json message;

			message["Info"] = "Unknown request type : " + std::to_string(uint32_t(reqType));

			response = createResponse(ResponseType::Error, false, std::move(message));
			
			break;
		}
		}
	}
	catch (const std::exception& e) {
		std::cout << "Exeption occured while parsing json: " << e.what() << std::endl;

		nlohmann::json message;

		message["Info"] = "Some of json properties incorrect: " + std::string(e.what());
		response = createResponse(ResponseType::Error, false, std::move(message));
	}

	return response;
}

nlohmann::json Session::handleSignUp(const nlohmann::json& clientMessage)
{
	bool status = false;
	nlohmann::json message;
	std::string login = clientMessage.at("Login");
	std::string password = clientMessage.at("Password");

	int64_t userID = database.registerNewUser(login, password);

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

	return createResponse(ResponseType::Registration, status, 
						  std::move(message));
}

nlohmann::json Session::handleSignIn(const nlohmann::json& clientMessage)
{
	bool status = false;
	nlohmann::json message;
	std::string login = clientMessage.at("Login");
	std::string password = clientMessage.at("Password");

	int64_t userID = database.getUserID(login, password);

	if (userID == -1) {
		status = false;
		message["Info"] = "This login doesn't exist";
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

	return createResponse(ResponseType::Registration, status, std::move(message));
}

nlohmann::json Session::handleTradeRequest(const nlohmann::json& clientMessage)
{
	nlohmann::json message;

	int64_t volume = clientMessage.at("Volume");
	int64_t price = clientMessage.at("Price");
	TradeRequestType reqType = clientMessage.at("TradeReqType");

	if (volume <= 0 || price <= 0) {
		message["Info"] = "Some of arguments incorrect, request denied";
		return createResponse(ResponseType::RequestResponse, false, 
							  std::move(message));
	}
	else {
		TradeRequest tradeRequest(clientID, volume, price, reqType);

		market.handleTradeRequest(tradeRequest);

		message["Info"] = "Trade request successfully registered";
		return createResponse(ResponseType::RequestResponse, true, 
							  std::move(message));
	}
}

nlohmann::json Session::handleInfoRequest()
{
	nlohmann::json message;
	nlohmann::json activeRequests = nlohmann::json::array();
	nlohmann::json tradeHistory = nlohmann::json::array();
	
	if (auto clientInfo = database.getClientInfo(clientID)) {
		//Client found
		auto [dollars, rubles] = clientInfo->getBalance();
		message["Balance"] = nlohmann::json::array();
		//Write user's balance in array
		message["Balance"].push_back(dollars);
		message["Balance"].push_back(rubles);
	}
	else {
		message["Info"] = "Client not found";
		return createResponse(ResponseType::ClientInfo, false, std::move(message));
	}

	//Construct user's active requests
	auto& activeRequestsList = market.getActiveRequests(clientID);

	if (!activeRequestsList.empty()) {
		for (auto& request : activeRequestsList) {
			activeRequests.push_back(request->createJsonObject());
		}
	}

	//Construct user's trade history
	auto requestsHistory = database.getClientTradeHistory(clientID);

	if (!requestsHistory.empty()) {
		for (auto& request : requestsHistory) {
			tradeHistory.push_back(request.createJsonObject());
		}
	}

	message["ActiveRequests"] = activeRequests;
	message["TradeHistory"] = tradeHistory;

	return createResponse(ResponseType::ClientInfo, true, std::move(message));
}

nlohmann::json Session::handleCancelRequest(const nlohmann::json& clientMessage)
{
	using boost::posix_time::ptime;
	using boost::posix_time::from_iso_extended_string;

	nlohmann::json response;
	bool status = false;

	int64_t price = clientMessage["Price"];
	int64_t volume = clientMessage["Volume"];
	TradeRequestType tradeType = clientMessage["Type"];

	ptime regTime = from_iso_extended_string(clientMessage["RegTime"].get<std::string>());

	TradeRequest reqToDelete(clientID, volume, price, regTime, tradeType);
	status = market.removeRequest(reqToDelete);

	if (status) {
		response["Info"] = "Request successfully canceled";
	}
	else {
		response["Info"] = "Request not found (already completed or deleted)";
	}

	return createResponse(ResponseType::CancelInfo, status, std::move(response));
}

nlohmann::json Session::createResponse(ResponseType type, bool status, nlohmann::json&& message)
{
	nlohmann::json response;

	response["ResponseType"] = type;
	response["Status"] = status;
	response.emplace("Message", std::move(message));

	return response;
}