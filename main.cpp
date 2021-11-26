

#include <iostream>
#include "Server.hpp"

int main(int ac, char *av[])
{
	Server *server;
	if (ac != 3)
	{
		std::cerr<<"usage: "<<av[0]<<" port password"<<std::endl;
		return (1);
	}
	try
	{
		server = new Server(av[1], av[2]);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error occured while creating server: " << e.what() << std::endl;
		return (1);
	}
	try
	{
		server->run();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error occured while the server was running: "<< e.what() << std::endl;
	}
	delete(server);
}