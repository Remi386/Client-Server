#include "Client.h"
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
	Client client;
	client.connect("127.0.0.1", std::to_string(PORT_NUM));
	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	client.sendMessage("test message");
	std::cout << "Received from server: " << client.readMessage() << std::endl;
	char c;
	std::cin >> c;
}