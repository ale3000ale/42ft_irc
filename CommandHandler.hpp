#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <string>
#include <list>

#include "User.hpp"

class Server;

class CommandHandler
{
	public:
		CommandHandler(Server	&server);

		void handle(std::string cmd_line, User& owner);

	private:
		typedef std::list<std::string>::iterator _iterator;

		std::string				_command;
		std::list<std::string>	_params;
		Server					&_server;

		void 					_parse_cmd(std::string cmd_line);
		void					_handlePASS(User &owner);
		void					_handleNICK(User& owner);
		void					_handleUSER(User& owner);
		void					_handlePING(User& owner);
		void					_handleJOIN(User &owner);
		void					_handlePART(User &owner);
		void					_handlePRIVMSG(User &owner);
		void					_handleAWAY(User &owner);
		void					_handleQUIT(User &owner);
		void 					_handleWHO(User& owner) const;
		void 					_handleMODE(User& owner) const;

		void					_welcome_msg(User& target) const;
	public:
		void					_numeric_reply(int val, User& owner, std::string extra = "") const;
};

#endif