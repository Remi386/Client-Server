#include "Session.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <boost/bind.hpp>

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
		reply.clear();

		sock.async_receive(asio::buffer(buffer, MAX_BUFFER_SIZE),
							boost::bind(&Session::readMessage, shared_from_this(),
										asio::placeholders::error,
										asio::placeholders::bytes_transferred));
	}
}

void Session::handleMessage()
{
	nlohmann::json request = nlohmann::json::parse(buffer);
	try {
		std::cout << "Message recieved: RequestType = "
				  << request.at("RequestType")
				  << ", UserID = " << request.at("UserID")
				  << ", Message body = " << request.at("Message")
				  << std::endl;
	}
	catch (const std::exception& e) {
		std::cout << "Exeption occured while parsing json: " << e.what() << std::endl;
	}

	nlohmann::json jsonReply;
	jsonReply["Message"] = "Message sended back: " + request.dump();
	reply = jsonReply.dump();
}