
#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <string>
#include <list>

#include "User.hpp"
#include "Server.hpp"

class Command
{
	public:
		Command(Server& server, User& owner, std::string cmd_line);

		void exec();

	private:
		std::string				_command;
		std::list<std::string>	_params;
		Server&					_server;
		User&					_owner;

		void					_handlePASS();
};

#endif