#include "Client.h"
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
	Client client;
	client.connect("127.0.0.1", std::to_string(PORT_NUM));
	
	client.loop();
}