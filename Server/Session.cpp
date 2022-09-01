#include "Session.h"
//#include <nlohmann/json.hpp>
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
		std::string message(&buffer[0], &buffer[bytes]);
		handleMessage(message);
		reply = "Message sended back: " + message;
		
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

void Session::handleMessage(const std::string& message)
{
	std::cout << "Message recieved: " << message << std::endl;
}