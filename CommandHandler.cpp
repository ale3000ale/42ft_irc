
#include "CommandHandler.hpp"
#include "Server.hpp"

CommandHandler::CommandHandler(Server	&server): _server(server) {}


void CommandHandler::_parse_cmd(std::string cmd_line)
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

void CommandHandler::handle(std::string cmd_line, User& owner)
{
	_parse_cmd(cmd_line);
	if (this->_command.empty())
		return ;
	if (!owner.is_passed() && this->_command != "PASS")
		return ; // TODO send numeric reply
	if (!owner.is_registered() && (this->_command != "NICK" || this->_command != "USER"))
		return ; // TODO send numeric reply
	// sarebbe figo avere una mappa chiave = comando(es PASS) valore = puntatore funzione corrispondente
	if (this->_command == "PASS")
		return (_handlePASS(owner));
	else if (this->_command == "JOIN")
		return (_handleJOIN(owner));
}

void CommandHandler::_handlePASS(User& owner)
{
	if (owner.is_registered())
		return ; //ERR_ALREADYREGISTERED (462)
	if (!this->_params.size())
		return ; //ERR_NEEDMOREPARAMS (461)
	if (this->_server.checkPass(this->_params.front()))
		owner.set_passed();
}

void CommandHandler::_handleJOIN(User& owner)
{
	if (_params.empty())
		return ; //ERR_NEEDMOREPARAMS (461)
	std::list<std::string> names;
	std::list<std::string> keys;
	int ck = 0;
	
	while( _params.front() != "")
	{
		names.push_back(_params.front().substr(0, _params.front().find(",")));
		_params.front().erase(0,  _params.front().find(",") + 1);
	}
	_params.pop_front();
	if (!_params.empty())
	{
		while( _params.front() != "")
			{
				keys.push_back(_params.front().substr(0, _params.front().find(",")));
				_params.front().erase(0,  _params.front().find(",") + 1);
			}
		_params.pop_front();
	}
	while(!names.empty())
	{
		if (_server.exist_channel(names.front()));
		{
			Channel &chan = _server.get_channel(names.front());
			if (keys.empty())
				ck = chan.join_user(owner);
			else
			{
				ck = chan.join_user(owner, keys.front());
				keys.pop_front();
			}
			
			names.pop_front();
			
			if (ck == 1)
				//TODO: send 3 standard msg
				std::cout << "join" << std::endl;
			else
				// TODO: send numeric msg
		}
		else
		{
			//TODO: create channel
		}
	}
	
}