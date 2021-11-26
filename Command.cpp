
#include "Command.hpp"

Command::Command(Server& server, User& owner, std::string cmd_line) : _server(server), _owner(owner)
{
	cmd_line = cmd_line.substr(0, cmd_line.length() - 2); //deleting trailing \r\n
	if (cmd_line.empty())
		return ;
	int pos = cmd_line.find(" ");
	this->_command = cmd_line.substr(0, pos);
	cmd_line.erase(0, (pos != -1) ? pos + 1 : pos);

	while (!cmd_line.empty())
	{
		if (cmd_line[0] == ':')
		{
			cmd_line.erase(0, 1);
			if (cmd_line.empty())
				break ;
			this->_params.push_back(cmd_line);
		}
		else
		{
			pos = cmd_line.find(" ");
			this->_params.push_back(cmd_line.substr(0, pos));
			cmd_line.erase(0, (pos != -1) ? pos + 1 : pos);
		}
	}
}

void Command::exec()
{
	if (this->_command.empty())
		return ;
	if (!this->_owner.is_passed() && this->_command != "PASS")
		return ; // TODO send numeric reply
	if (!this->_owner.is_registered() && (this->_command != "NICK" || this->_command != "USER"))
		return ; // TODO send numeric reply
	// sarebbe figo avere una mappa chiave = comando(es PASS) valore = puntatore funzione corrispondente
	if (this->_command == "PASS")
		return (_handlePASS());
}

void Command::_handlePASS()
{
	if (this->_owner.is_registered())
		return ; //ERR_ALREADYREGISTERED (462)
	if (!this->_params.size())
		return ; //ERR_NEEDMOREPARAMS (461)
	if (this->_server.checkPass(this->_params.front()))
		this->_owner.set_passed();
}