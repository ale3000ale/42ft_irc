
#ifndef BOT_HPP
# define BOT_HPP

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>

class Bot
{
	public:
		Bot(std::string host_ip, std::string port, std::string password);
		~Bot();

		void run();

	private:
		std::string _server_ip;
		std::string _port;
		std::string _password;
		int			_socket_fd;
		char		_buff[512];

		int			_register();
		int			_get_numeric(std::string cmd) const;
};

#endif