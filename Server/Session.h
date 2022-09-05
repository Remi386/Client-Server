#pragma once
#include <memory>
#include <string>
#include <deque>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include "../NetCommon/NetCommon.h"

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& context)
		: sock(context)
	{}

	~Session();

	boost::asio::ip::tcp::socket& socket() { return sock; }

	void start();

	void sendNotification(const std::string& content);

private:
	//All *Task methods register async tasks in asio context

	void readMessageTask();

	void sendMessageTask();

	void sendNotificationTask();

	//All *Handler methods invokes when appropriate event occured, e.g. message sended

	void readMessageHandler(const boost::system::error_code& error, size_t bytes);

	void sendMessageHandler(const boost::system::error_code& error);

	void sendNotificationHandler(const boost::system::error_code& error);

	/// <summary>
	/// Handle message from buffer
	/// </summary>
	void handleMessage();

	//Helper functions to handle message

	void handleSignUp(const nlohmann::json& message);

	void handleSignIn(const nlohmann::json& message);

	void handleTradeRequest(const nlohmann::json& message);

	void handleInfoRequest();

	void handleCancelRequest(const nlohmann::json& message);

	/// <summary>
	/// This method called when user gets his ID, after signing up/in
	/// </summary>
	void addSessionToActive();

	void createResponse(ResponseType type, bool status, nlohmann::json&& message);

private:
	//We should keep reply buffer valid, so we will store data in string
	std::string reply;
	std::deque<std::string> notificationBuffers;
	boost::asio::ip::tcp::socket sock;
	int64_t clientID;
	boost::asio::streambuf buffer;
	//char buffer[MAX_BUFFER_SIZE];
};