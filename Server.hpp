
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
#include "Channel.hpp"
#include "CommandHandler.hpp"
#include <map>

#define BACKLOG 10 // number of connections allowed on the incoming queue


class Server
{
	public:
		Server(std::string port, std::string password);
		~Server();

		void 			run();
		bool 			checkPass(std::string &pass);
		bool			exist_channel(std::string name);
		bool			add_channel(Channel ch);
		Channel			&get_channel(std::string name);
		std::vector<User> const & getUserList() const;
		void			send_msg(std::string& msg, User const & target);
		void 			deleteUser(std::string nickname);
		CommandHandler	getHendler() const;

	private:
		std::string						_port;
		std::string						_password; // dont know if  needed
		int								_socket_fd;
		std::vector<struct pollfd>		_pfds;
		std::vector<User>				_users;
		std::map<std::string, Channel>	_channels;
		CommandHandler					_handler;

		void						_addUser();
		void 						_deleteUser(int index);
		void						_addFd(int new_fd);
		void 						_exec_cmd(User& executor);
};

#endif