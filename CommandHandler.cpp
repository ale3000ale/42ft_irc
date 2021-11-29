
#include "CommandHandler.hpp"
#include "Server.hpp"

CommandHandler::CommandHandler(Server	&server): _server(server) {}


void CommandHandler::_parse_cmd(std::string cmd_line)
{
	//cmd_line = cmd_line.substr(0, cmd_line.length() - 2); //deleting trailing \r\n
	if (cmd_line.empty())
		return ;
	int pos = cmd_line.find(" ");
	this->_command = cmd_line.substr(0, pos);
	cmd_line.erase(0, (pos != -1) ? pos + 1 : pos);
	this->_params.clear();

	while (!cmd_line.empty())
	{
		if (cmd_line[0] == ':')
		{
			cmd_line.erase(0, 1);
			if (cmd_line.empty())
				break ;
			this->_params.push_back(cmd_line);
			cmd_line.erase(0, -1);
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
	std::cout<<"command: "<<cmd_line<<std::endl;
	_parse_cmd(cmd_line);
	if (this->_command.empty())
		return ;
	if (!owner.is_passed() && this->_command != "PASS")
		return ; // TODO send numeric reply
	else if (owner.is_passed() && !owner.is_registered() && this->_command != "NICK" && this->_command != "USER")
		return _numeric_reply(451, owner, this->_command); // ERR_NOTREGISTERED
	// sarebbe figo avere una mappa chiave = comando(es PASS) valore = puntatore funzione corrispondente
	if (this->_command == "PASS")
		_handlePASS(owner);
	else if (this->_command == "NICK")
		_handleNICK(owner);
	else if (this->_command == "USER")
		_handleUSER(owner);
	else if (this->_command == "PING")
		_handlePING(owner);
	else if (this->_command == "JOIN")
		_handleJOIN(owner);
	else if (this->_command == "PRIVMSG")
		_handlePRIVMSG(owner);
	else if (this->_command == "QUIT")
		_handleQUIT(owner);
	else
		_numeric_reply(421, owner, this->_command); // ERR_UNKNOWNCOMMAND
}

void CommandHandler::_handlePASS(User& owner)
{
	if (!this->_params.size())
		return _numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS
	if (owner.is_registered())
		return _numeric_reply(462, owner); //ERR_ALREADYREGISTERED
	if (this->_server.checkPass(this->_params.front()))
		owner.set_passed();
}

void CommandHandler::_handleNICK(User& owner)
{
	if (!this->_params.size())
		return _numeric_reply(431, owner); // ERR_NONICKNAMEGIVEN (431)
	std::string& nick = this->_params.front();
	std::vector<User> const & users = this->_server.getUserList();
	for (u_int i = 0; i < users.size(); i++)
	{
		if (users[i].getNick() == nick)
			return _numeric_reply(433, owner, nick); ; // ERR_NICKNAMEINUSE (433)
	}
	owner.setNick(nick);
	_numeric_reply(1, owner);
	/* 
	 MAY ADD NUMERIC REPLY 2 3 4 5
	*/
}

void CommandHandler::_handleUSER(User& owner)
{
	/*for (std::list<std::string>::iterator i =_params.begin(); i != _params.cend() ; i++)
		std::cout<<*i<<" --- ";*/
	if (this->_params.size() != 4)
		return _numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS
	if (owner.is_registered())
		return _numeric_reply(462, owner); //ERR_ALREADYREGISTERED
	std::string username = this->_params.front();
	if (username.empty()) // i think this case wont ever occur
		return _numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS (461)
	std::string realname = this->_params.back();
	owner.setUsername(username);
	owner.setRealname(realname);
	owner.set_registered();
	_numeric_reply(1, owner); // RPL_WELCOME
}

void CommandHandler::_handlePING(User& owner)
{
	if (!this->_params.size())
		return _numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS
	//std::cout<<"PONG TEST: "<<this->_params.front()<<std::endl;
	std::string msg = ":myIRCServer PONG myIRCServer :" + this->_params.front() + "\r\n";
	this->_server.send_msg(msg, owner);
}

void CommandHandler::_handlePRIVMSG(User& owner)
{
	if (!this->_params.size())
		return ; // ERR_NORECIPIENT (411)
	if (this->_params.size() == 1)
		return ; // ERR_NOTEXTTOSEND (412)
	std::string text = this->_params.back();
	// TODO
}

void CommandHandler::_handleJOIN(User& owner)
{
	std::string test = owner.getNick();
	/*if (_params.empty())
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
	}*/
}

void CommandHandler::_handleQUIT(User& owner)
{
	std::string reason = (_params.size() == 1) ? _params.front() : owner.getNick();
	std::string msg = "ERROR :Closing Link: " + owner.getNick() + "[" + owner.getHost() + "] (Quit: " + reason + ")\r\n";
	this->_server.send_msg(msg, owner);
	/*
		MAY ADD SENDING MESSAGE TO OTHER CLIENTS THAT SHARE CHANNEL WITH EXITING USER
	*/
}

void	CommandHandler::_numeric_reply(int val, User& owner, std::string extra = "")
{
	std::string msg = ":myIRCServer ";

	switch (val)
	{
		case 1: // RPL_WELCOME
			msg += "001 " + owner.getNick() + " :Welcome to the Internet Relay Network ";
			msg += owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost();
			break;
		case 421: // ERR_UNKNOWNCOMMAND 
			msg += "421 " + owner.getNick() + " " + extra + " :Unknown command";
			break;
		case 431: // ERR_NONICKNAMEGIVEN
			msg += "431 " + owner.getNick() + " :No nickname given";
			break;
		case 433: // ERR_NICKNAMEINUSE 
			msg += "433 " + owner.getNick() + " " + extra + " :Nickname is already in use"; 
			break;
		case 451: // ERR_NOTREGISTERED
			msg += "451 " + extra + " :You have not registered";
			break;
		case 461: // ERR_NEEDMOREPARAMS 
			msg += "461 " + owner.getNick() + " " + extra + " :Not enough parameters"; 
			break;
		case 462: // ERR_ALREADYREGISTERED
			msg += "462 " + owner.getNick() + " :You may not reregister"; 
			break;
		default:
			break;
	}
    msg += "\r\n";
	//std::cout<<msg;
	this->_server.send_msg(msg, owner);
}