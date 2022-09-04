#include "Client.h"
#include <iostream>

int main()
{
	Client client;

	client.connect("127.0.0.1", std::to_string(PORT_NUM));
	
	client.loop();
}