#pragma once
#include <memory>
#include <string>
#include <deque>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include "../NetCommon/NetCommon.h"

class Marketplace;
class DataBase;

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& context, Marketplace& market_, DataBase& database_)
		: sock(context), market(market_), database(database_)
	{}

	~Session();

	boost::asio::ip::tcp::socket& socket() { return sock; }

	void start();

	void sendNotification(const std::string& content);

	/// <summary>
	/// Handle message from client
	/// </summary>
	/// <param name="message">rvalue of json object, what prevents copies</param>
	/// <returns>Response in json format</returns>
	nlohmann::json handleRequest(nlohmann::json&& message);

private:
	//All *Task methods register async tasks in asio context

	void readMessageTask();

	void sendMessageTask();

	void sendNotificationTask();

	//All *Handler methods invokes when appropriate event occured, e.g. message sended

	void readMessageHandler(const boost::system::error_code& error, size_t bytes);

	void sendMessageHandler(const boost::system::error_code& error);

	void sendNotificationHandler(const boost::system::error_code& error);

	//Helper functions to handle message

	nlohmann::json handleSignUp(const nlohmann::json& message);

	nlohmann::json handleSignIn(const nlohmann::json& message);

	nlohmann::json handleTradeRequest(const nlohmann::json& message);

	nlohmann::json handleInfoRequest();

	nlohmann::json handleCancelRequest(const nlohmann::json& message);

	/// <summary>
	/// This method called when user gets his ID, after signing up/in
	/// </summary>
	void addSessionToActive();

	/// <summary>
	/// Helper method to create response, creates required fields in json
	/// </summary>
	/// <param name="type">Type of response</param>
	/// <param name="status">Status of operation - success or failure</param>
	/// <param name="message">Inner object with necessary information</param>
	nlohmann::json createResponse(ResponseType type, bool status, nlohmann::json&& message);

private:
	//Dependency injection
	Marketplace& market;
	DataBase& database;

	//We should keep reply buffer valid, so we will store data in string
	std::string reply;
	//Read buffer
	boost::asio::streambuf buffer;

	//We send notifications separately, so we keep all of them
	std::deque<std::string> notificationBuffers;

	boost::asio::ip::tcp::socket sock;
	int64_t clientID = -1;
};