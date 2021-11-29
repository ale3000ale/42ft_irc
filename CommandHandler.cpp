
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
	/*if (!owner.is_passed() && this->_command != "PASS")
		return ; // TODO send numeric reply
	if (!owner.is_registered() && (this->_command != "NICK" || this->_command != "USER"))
		return ; // TODO send numeric reply*/
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
	this->_params.clear();
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

void CommandHandler::_handleNICK(User& owner)
{
	bool welcomed = true;
	if (!this->_params.size())
		return ; // ERR_NONICKNAMEGIVEN (431)
	if (owner.getNick().empty())
		welcomed = false;
	std::string& nick = this->_params.front();
	std::vector<User> const & users = this->_server.getUserList();
	for (u_int i = 0; i < users.size(); i++)
	{
		if (users[i].getNick() == nick)
			return ; // ERR_NICKNAMEINUSE (433)
	}
	owner.setNick(nick);
	if (!welcomed)
		_numeric_reply(1, owner);
}

void CommandHandler::_handleUSER(User& owner)
{
	if (owner.is_registered())
		return ; //ERR_ALREADYREGISTERED (462)
	/*for (std::list<std::string>::iterator i =_params.begin(); i != _params.cend() ; i++)
		std::cout<<*i<<" --- ";*/
	if (this->_params.size() != 4)
		return ; // ERR_NEEDMOREPARAMS (461)
	std::string username = this->_params.front();
	if (username.empty()) // i think this case wont ever occur
		return ; // ERR_NEEDMOREPARAMS (461)
	std::string realname = this->_params.back();
	owner.setUsername(username);
	owner.setRealname(realname);
	owner.set_registered();
	if (!owner.getNick().empty())
		_numeric_reply(1, owner); // RPL_WELCOME
}

void CommandHandler::_handlePING(User& owner)
{
	if (!this->_params.size())
		return ; // ERR_NONICKNAMEGIVEN (431)
	//std::cout<<"PONG TEST: "<<this->_params.front()<<std::endl;
	std::string msg = ":myIRCServer PONG myIRCServer :" + this->_params.front() + "\r\n";
	this->_server.send_msg(msg, owner);
}

void CommandHandler::_handleJOIN(User& owner)
{
	
	std::string test = owner.getNick();
	if (_params.empty())
		return ; //ERR_NEEDMOREPARAMS (461)
	std::list<std::string> names;
	std::list<std::string> keys;
	int pos;
	int ck = 0;
	
	
	while( _params.front() != "")
	{
		pos = _params.front().find(",");
		names.push_back(_params.front().substr(0, pos));
		_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
	}
	
	
	_params.pop_front();
	if (!_params.empty())
	{
		while( _params.front() != "")
			{
				pos = _params.front().find(",");
				keys.push_back(_params.front().substr(0, pos));
				_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
			}
		_params.pop_front();
	}
	std::cout << "NAMES: ";
	for (std::list<std::string>::iterator i = names.begin(); i != names.end() ; i++)
		std::cout << *i ;
	std::cout	<<std::endl;

	std::cout << "KEYS:  ";
	for (std::list<std::string>::iterator i = keys.begin(); i != keys.end() ; i++)
		std::cout << *i ;
	std::cout	<<std::endl;


	while(!names.empty())
	{
		std::cout << "EXIST?: "<< std::endl;
		if (!_server.exist_channel(names.front()))
		{
			std::cout << "CREATE?: "<< std::endl;
			Channel ch(names.front(), keys.front(), owner);
			_server.add_channel(ch);
			std::cout << "ADDED?: "<< std::endl;
		}

		std::cout << "JOINING"<< std::endl;
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
		{
			std::string msg = owner.getUsername() + "!" +  owner.getUsername() + "@" + owner.getHost() + " JOIN :" + chan.getName() + "\r\n";
			_server.send_msg(msg, owner);
			std::cout << "join" << std::endl;
		}
		else
		{
			// TODO: send numeric msggit 
			continue;
		}
	}
}

void	CommandHandler::_numeric_reply(int val, User& owner, std::string ch = "")
{
	std::string msg = ":myIRCServer ";

	switch (val)
	{
		case 1: // RPL_WELCOME
			msg += "001 " + owner.getNick() + " :Welcome to the Internet Relay Network ";
			break;
		case 353: // RPL_NAMREPLY
			msg += "353 " + owner.getNick() + " =" + ch + " :";
			msg += _server.get_channel(ch).getStrUsers();
			break;
		case 366: // RPL_WELCOME
			msg += "366 " + owner.getNick() + " :Welcome to the Internet Relay Network ";
			break;
		default:
			break;
	}
	msg += owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost() + "\r\n";
	//std::cout<<msg;
	this->_server.send_msg(msg, owner);
}