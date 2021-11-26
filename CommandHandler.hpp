#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <string>
#include <list>

#include "Server.hpp"

class CommandHandler
{
	public:
		CommandHandler(Server	&server);

		void handle(std::string cmd_line, User& owner);

	private:
		std::string				_command;
		std::list<std::string>	_params;
		Server					&_server;

		void 					_parse_cmd(std::string cmd_line);
		void					_handlePASS(User &owner);
		void					_handleJOIN(User &owner);
};

#endif