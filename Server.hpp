
#ifndef SERVER_HPP
# define SERVER_HPP

#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdexcept>
#include <cerrno>
#include <unistd.h>
#include <vector>
#include <poll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstdio>

#include "User.hpp"

#define BACKLOG 10 // number of connections allowed on the incoming queue

class Server
{
	public:
		Server(std::string port, std::string password);
		~Server();

		void run();
	private:
		std::string					_port;
		std::string					_password;
		int							_socket_fd;
		std::vector<struct pollfd>	_pfds;
		std::vector<User>			_users;

		void						_addUser();
		void 						_deleteUser(int index);
		void						_addFd(int new_fd);
};

#endif