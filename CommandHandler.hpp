#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <string>
#include <list>

#include "User.hpp"

class CommandHandler
{
	public:
		CommandHandler(std::string server_pass);

		void handle(std::string cmd_line, User& owner);

	private:
		std::string				_command;
		std::list<std::string>	_params;
		std::string				_server_pass;

		void 					_parse_cmd(std::string cmd_line);
		void					_handlePASS(User &owner);
};

#endif