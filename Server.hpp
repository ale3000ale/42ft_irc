
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
		bool checkPass(std::string &pass);
	private:
		std::string					_port;
		std::string					_password; // dont know if  needed
		int							_socket_fd;
		std::vector<struct pollfd>	_pfds;
		std::vector<User>			_users;
		CommandHandler				_handler;

		void						_addUser();
		void 						_deleteUser(int index);
		void						_addFd(int new_fd);
		void 						_exec_cmd(User& executor);
};

#endif