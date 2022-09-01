#pragma once
#include <boost/asio.hpp>
#include <string>
#include <cstdint>
#include "../NetCommon/NetCommon.h"

class Client {
public:
	Client();

	void connect(const std::string& adress, const std::string& port);

	std::string readMessage();

	void sendMessage(const std::string& message);

private:
	boost::asio::io_context context;
	boost::asio::ip::tcp::socket sock;
	uint32_t clientID;
};