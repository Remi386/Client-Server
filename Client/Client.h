#pragma once
#include <boost/asio.hpp>
#include <string>
#include <nlohmann/json.hpp>
#include <thread>
#include <list>
#include "../NetCommon/NetCommon.h"
#include "../NetCommon/TradeRequest.h"

class Client {
public:
	Client();

	~Client();

	void connect(const std::string& adress, const std::string& port);

	void loop();

private:
	void readMessage(const boost::system::error_code& error,
					 size_t bytes);

	void sendMessage(RequestType type, const nlohmann::json& message);

	void handleRegistrationRequest(RequestType type);

	void handleTradeRequest();

	void handleCancelRequest();

	void handleResponse(const nlohmann::json& response);

	void handleGetInfoResponse(bool status, const nlohmann::json& message);

private:
	std::thread clientThread;
	boost::asio::io_context context;
	boost::asio::ip::tcp::socket sock;
	boost::asio::streambuf buffer;
	
	int64_t clientID = -1;
	std::list<TradeRequest> clientRequests;
};