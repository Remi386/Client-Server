#pragma once
#include <memory>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <string>

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

	void handleMessage(const std::string& message);

private:
	std::string reply;
	boost::asio::ip::tcp::socket sock;
	char buffer[MAX_BUFFER_SIZE];
};