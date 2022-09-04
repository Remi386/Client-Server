#pragma once
#include <boost/asio.hpp>
#include <string>
#include <nlohmann/json.hpp>
#include <thread>
#include "../NetCommon/NetCommon.h"

class Client {
public:
	Client();

	void connect(const std::string& adress, const std::string& port);

	void loop();

private:
	void readMessage(const boost::system::error_code& error,
					 size_t bytes);

	void sendMessage(RequestType type, const nlohmann::json& message);

	void handleRegistrationRequest(RequestType type);

	void handleTradeRequest();

	void handleResponse(const nlohmann::json& response);

	void handleGetInfoResponse(bool status, const nlohmann::json& message);

	std::thread clientThread;
	boost::asio::io_context context;
	boost::asio::ip::tcp::socket sock;
	int64_t clientID = -1;
	//std::string buffer;
	boost::asio::streambuf buffer;
};