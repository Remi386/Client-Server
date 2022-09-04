#pragma once
#include <memory>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include "../NetCommon/NetCommon.h"

inline constexpr int MAX_BUFFER_SIZE = 1024;

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& context)
		: sock(context)
	{}

	boost::asio::ip::tcp::socket& socket() { return sock; }

	void start();

private:
	void readMessage(const boost::system::error_code& error, size_t bytes);

	void sendMessage(const boost::system::error_code& error);

	void handleMessage();

	void handleSignUp(const nlohmann::json& message);

	void handleSignIn(const nlohmann::json& message);

	void handleTradeRequest(int64_t userID, RequestType type, 
							const nlohmann::json& message);

	void handleInfoRequest(int64_t userID);

private:
	//We should keep reply buffer valid, so we will store data in string
	std::string reply;
	boost::asio::ip::tcp::socket sock;
	char buffer[MAX_BUFFER_SIZE];
};