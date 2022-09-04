#pragma once
#include <boost/asio.hpp>
#include <string>
#include <cstdint>
#include "../NetCommon/NetCommon.h"
#include <nlohmann/json.hpp>

class Client {
public:
	Client();

	void connect(const std::string& adress, const std::string& port);

	void loop();

private:
	std::string readMessage();

	void sendMessage(RequestType type, const nlohmann::json& message);

	void handleRegistration(RequestType type);

	void handleTradeRequest();

	void handleGetInfo();

	boost::asio::io_context context;
	boost::asio::ip::tcp::socket sock;
	int64_t clientID = -1;
};