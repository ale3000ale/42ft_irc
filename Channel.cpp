#include "Channel.hpp"
#include "Server.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Channel::Channel()
{
	_creationTime = std::time(nullptr);
}

Channel::Channel(std::string name, Server &server) : 
	_name(name), _key(""), _topic(""), _server(&server)
{
	_creationTime = std::time(nullptr);
	_topicTime = std::time(nullptr);
	_limit = INT32_MAX;
}

Channel::Channel(std::string name, std::string key, Server &server): 
	_name(name), _key(key), _topic(""), _server(&server)/*, _founder(us)*/
{
	_creationTime = std::time(nullptr);
	_topicTime = std::time(nullptr);
	_limit = INT32_MAX;
}

Channel::Channel(Channel const & ch): 
	_name(ch._name), _key(ch._key), _topic(ch._topic), _server(ch._server), _users(ch._users)
{
	_creationTime = ch._creationTime;
	_topicTime = ch._topicTime;
	_limit = INT32_MAX;
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Channel::~Channel()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

/* Channel &				Channel::operator=( Channel const & rhs )
{
	if ( this != &rhs )
	{

		this->_value = rhs.getValue();
	}
	return *this;
} */

/*std::ostream &			operator<<( std::ostream & o, Channel const & i )
{
	//o << "Value = " << i.getValue();
	return o;
}*/


/*
** --------------------------------- METHODS ----------------------------------
*/

void				Channel::ban(User &owner, std::string nick)
{
	if (!this->isOperator(owner))
	{
		std::cout << "NOT OPERATOR\n";
		return (_server->getHandler()._numeric_reply(482, owner, _name));
	}
	if ((_banList.find(nick) != _banList.end()))
		return;
	_banList.insert(nick);
	std::cout << "NON TROVATO NELLA BAN LIST\n";
	std::string msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + " +b :" + nick + "!*@*" + "\r\n";
	if (this->_modes.find('b') == std::string::npos)
		_modes += "b";
	this->sendAll(msg);
}

void				Channel::unBan(User &owner, std::string nick)
{
	if (!this->isOperator(owner))
		return (_server->getHandler()._numeric_reply(482, owner, _name));
	if (_banList.find(nick) == _banList.end())
		return;
	_banList.erase(nick);
	std::string msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + " -b :" + nick + "!*@*" + "\r\n";
	this->sendAll(msg);
	if (_banList.empty())
		this->delMode('b');
}
void				Channel::exception(User &owner, std::string nick, char type)
{
	std::string msg;
	if (!this->isOperator(owner))
		return (_server->getHandler()._numeric_reply(482, owner, _name));
	switch (type)
	{
	case 'I':
			std::cout << "IIIII\n";
			if (_excInviteList.find(nick) != _excInviteList.end())
				return;
			_excInviteList.insert(nick);
			std::cout << "inserted IIIIII\n";
			msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
								" MODE " + _name + " +I :" + nick + "!*@*" + "\r\n";
			if (this->_modes.find('I') == std::string::npos)
				_modes += "I";
			this->sendAll(msg);
		break;
	
	case 'e':
			if (_excBanList.find(nick) != _excBanList.end())
				return;
			_excBanList.insert(nick);
			msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
								" MODE " + _name + " +e :" + nick + "!*@*" + "\r\n";
			this->sendAll(msg);
			if (this->_modes.find('e') == std::string::npos)
				_modes += "e";
		break;
	}
}

void				Channel::unException(User &owner, std::string nick, char type)
{
	std::string msg;
	if (!this->isOperator(owner))
		return (_server->getHandler()._numeric_reply(482, owner, _name));
	switch (type)
	{
	case 'I':
			if (_excInviteList.find(nick) == _excInviteList.end())
				return;
			_excInviteList.erase(nick);
			msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
								" MODE " + _name + " -I :" + nick + "!*@*" + "\r\n";
			this->sendAll(msg);
			if (_excInviteList.empty())
				this->delMode('I');
		break;
	
	case 'e':
			if (_excBanList.find(nick) == _excBanList.end())
				return;
			_excBanList.erase(nick);
			msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
								" MODE " + _name + " -e :" + nick + "!*@*" + "\r\n";
			this->sendAll(msg);
			if (_excBanList.empty())
				this->delMode('e');
		break;
	}
}

bool				Channel::empty()
{
	return _users.empty();
}


void				Channel::delMode(char mode)
{
	int pos;
	if ((pos = this->_modes.find(mode)) == -1)
		return ;
	std::cout << "DELMODE " +this->_modes.substr(0, pos)+ " -- "+ this->_modes.substr(pos+1) + "\n";
	this->_modes = this->_modes.substr(0, pos) + this->_modes.substr(pos+1);
}

//TODO: other MODE
bool				Channel::addMode(User &owner, char m, char mode, std::string param)
{
	if (!mode)
		mode = '+';
	std::cout << "MODE " +std::string(1, mode)+ std::string(1, m) + " PARAMS "+ param + "\n";
	switch (m)
	{
		case 'b':
			return modeBAN(owner, mode, param);
		case 'e':
			return modeEXCBAN(owner, mode, param);
		case 'I':
			return modeEXCINVITE(owner, mode, param);
		case 'i':
			return modeINVITE(owner, mode);
		case 'k':
			return modeKEY(owner, mode, param);
		case 'l':
			return modeLIMIT(owner, mode, param);
		case 'o':
			return modeOPERATOR(owner, mode, param);
		case 'm':
			return modeMODERATE(owner, mode);
		case 's':
			return modeSECRET(owner, mode);
		case 't':
			return modeTOPIC(owner, mode);
		case 'n':
			return modeNOBURINI(owner, mode);
	}
	return false;
}

bool				Channel::modeNOBURINI(User &owner, char mode)
{
	std::string msg;
	std::cout << "NOBURINI\n";
	if (!this->isOperator(owner))
		_server->getHandler()._numeric_reply(482, owner, _name);
	else if (mode == '+')
	{
		if (this->_modes.find('n') != std::string::npos)
			return false;
		_modes += "n";
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +n" + "\r\n";
		this->sendAll(msg);	
	}
	else
	{
		if (this->_modes.find('n') == std::string::npos)
			return false;
		this->delMode('n');
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -n" + "\r\n";
		this->sendAll(msg);
	}
	return false;
}

bool				Channel::modeTOPIC(User &owner, char mode)
{
	std::string msg;
	std::cout << "SECRET\n";
	if (!this->isOperator(owner))
		_server->getHandler()._numeric_reply(482, owner, _name);
	else if (mode == '+')
	{
		if (this->_modes.find('t') != std::string::npos)
			return false;
		_modes += "t";
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +t" + "\r\n";
		this->sendAll(msg);	
	}
	else
	{
		if (this->_modes.find('t') == std::string::npos)
			return false;
		this->delMode('t');
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -t" + "\r\n";
		this->sendAll(msg);
	}
	return false;
}

bool				Channel::modeSECRET(User &owner, char mode)
{
	std::string msg;
	std::cout << "SECRET\n";
	if (!this->isOperator(owner))
		_server->getHandler()._numeric_reply(482, owner, _name);
	else if (mode == '+')
	{
		if (this->_modes.find('s') != std::string::npos)
			return false;
		_modes += "s";
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +s" + "\r\n";
		this->sendAll(msg);	
	}
	else
	{
		if (this->_modes.find('s') == std::string::npos)
			return false;
		this->delMode('s');
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -s" + "\r\n";
		this->sendAll(msg);
	}
	return false;
}

bool				Channel::modeMODERATE(User &owner, char mode)
{
	std::string msg;
	std::cout << "MODERATE\n";
	if (!this->isOperator(owner))
		_server->getHandler()._numeric_reply(482, owner, _name);
	else if (mode == '+')
	{
		if (this->_modes.find('m') != std::string::npos)
			return false;
		_modes += "m";
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +m" + "\r\n";
		this->sendAll(msg);	
	}
	else
	{
		if (this->_modes.find('m') == std::string::npos)
			return false;
		this->delMode('m');
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -m" + "\r\n";
		this->sendAll(msg);
	}
	return false;
}


bool				Channel::modeINVITE(User &owner, char mode)
{
	std::string msg;
	std::cout << "INVITE\n";
	if (!this->isOperator(owner))
		_server->getHandler()._numeric_reply(482, owner, _name);
	else if (mode == '-' && _modes.find('i') != std::string::npos)
	{
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -i" + "\r\n";
		this->sendAll(msg);
		this->delMode('i');
	}
	else if (_modes.find('i') == std::string::npos)
	{
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +i" + "\r\n";
		this->sendAll(msg);
		if (this->_modes.find('i') == std::string::npos)
			_modes += "i";
	}
	return false;
}

bool				Channel::modeBAN(User &owner, char mode, std::string param)
{
	std::string msg;
	std::cout << "BAN\n";
	if (param == "" && mode == '-')
		return false;
	else if (param == "")
	{
		sendBanList(owner);
		return false;
	}
	else if (mode == '+')
		ban(owner, param);
	else
		unBan(owner, param);
	return true;
}

bool				Channel::modeEXCINVITE(User &owner, char mode, std::string param)
{
	std::string msg;
	std::cout << "EXE_INV\n";
	if (param == "" && mode == '-')
		return false;
	else if (param == "")
	{
		sendExeInviteList(owner);
		return false;
	}
	else if (mode == '+')
		exception(owner, param, 'I');
	else
		unException(owner, param, 'I');
	return true;
}

bool				Channel::modeEXCBAN(User &owner, char mode, std::string param)
{
	std::string msg;
	std::cout << "EXE_BAN\n";
	if (param == "" && mode == '-')
		return false;
	else if (param == "")
	{
		sendExeBanList(owner);
		return false;
	}
	else if (mode == '+')
		exception(owner, param, 'e');
	else
		unException(owner, param, 'e');
	return true;
}

bool				Channel::modeOPERATOR(User &owner, char mode, std::string param)
{
	std::string msg;
	std::cout << "SET OPERATOR\n";
	if (!this->isOperator(owner))
		_server->getHandler()._numeric_reply(482, owner, _name);
	else if (param == "")
		return false;
	else 
	{
		std::cout << "param not empty\n";
		size_t i = 0;
		for (; i< _users.size(); i++)
			if (_users[i].second->getNick() == param)
				break;
		std::cout << "SIZE " <<i << "\n";
		if (i < _users.size())
		{
			if (mode == '+')
				_users[i].first = '@';
			else
				_users[i].first = '\0';
			msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + " " + mode +"o " + param +"\r\n";
			this->sendAll(msg);
		}
		else
			_server->getHandler()._numeric_reply(401, owner, param);
	}
	return true;
}

bool				Channel::modeLIMIT(User &owner, char mode, std::string param)
{
	std::string msg;
	std::cout << "LIMIT\n";
	if (!this->isOperator(owner))
		_server->getHandler()._numeric_reply(482, owner, _name);
	else if (mode == '+' && param != "")
	{
		int limit = std::atoi(param.c_str());
		if (limit)
		{
			_limit = limit;
			msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + " +l :" + std::to_string(_limit) + "\r\n";
			this->sendAll(msg);
			if (this->_modes.find('l') == std::string::npos)
				_modes += "l";
		}
		return (true);
	}
	else if (mode == '-' && this->_modes.find('l') == std::string::npos)
	{
		this->delMode('l');
		_limit = INT32_MAX;
		msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + ": -l\r\n";
		this->sendAll(msg);
	}
	return false;			
}
bool				Channel::modeKEY(User &owner, char mode, std::string param)
{
	std::string msg;
	std::cout << "KEY\n";
	if (!this->isOperator(owner))
		_server->getHandler()._numeric_reply(482, owner, _name);
	if (param == "" )
	{
		msg = _name + " k * :You must specify a parameter.";
		_server->getHandler()._numeric_reply(696,owner, msg);
		return false;
	}
	else if (mode == '-')
	{
		if (param == _key)
		{
			_key = "";
			this->delMode('k');
			msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -k :"+ param + "\r\n";
			this->sendAll(msg);
		}
		return true;
	}
	else
	{
		if (_key == "")
		{
			msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +k :"+ param + "\r\n";
			this->sendAll(msg);
			if (this->_modes.find('k') == std::string::npos)
				_modes += "k";
		}
		return true;
	}
}



void				Channel::sendBanList(User &owner) 		const
{
	std::string msg;
	for (iter_excIB_list i = _banList.begin(); i != _banList.end();i++)
	{
		std::string msg = _name + " " + *i;
		_server->getHandler()._numeric_reply(367, owner, msg);
	}
	_server->getHandler()._numeric_reply(368, owner, _name);

}
void				Channel::sendExeInviteList(User &owner)	const
{
	std::string msg;
	for (iter_excIB_list i = _excInviteList.begin(); i != _excInviteList.end();i++)
	{
		std::string msg = _name + " " + *i;
		_server->getHandler()._numeric_reply(346, owner, msg);
	}
	_server->getHandler()._numeric_reply(347, owner, _name);
}

void				Channel::sendExeBanList(User &owner)	const
{
	std::string msg;
	for (iter_excIB_list i = _excBanList.begin(); i != _excBanList.end();i++)
	{
		std::string msg = _name + " " + *i;
		_server->getHandler()._numeric_reply(348, owner, msg);
	}
	_server->getHandler()._numeric_reply(349, owner, _name);
}

int			Channel::join_user(User &user, std::string key , char status = 0)
{
	if (this->isInChannel(user))
	{
		std::cout << "User: " + user.getNick() + ":" + user.getUsername() + " is already in the channel!\n";
		return (0);
	}
	std::cout << "PASS: " + _key + "\n";
	if (key == _key)
	{
		user.addChannel(_name);
		_users.push_back(std::pair<char,User *>(status, &user));
		std::string msg = ":" + user.getNick() + "!" +  user.getUsername() + '@' + user.getHost() + " JOIN :" + _name + "\r\n";
		this->sendAll(msg);
		if (!_topic.empty())
			_server->getHandler()._numeric_reply(332, user, _name);
		_server->getHandler()._numeric_reply(353, user, "= "+_name + " :"+ this->getStrUsers());
		_server->getHandler()._numeric_reply(366, user, _name);
		return (1);
	}
	else
		_server->getHandler()._numeric_reply(475, user, _name);
	return(475);	//ERR_BADCHANNELKEY (475)
}

void	Channel::part_user(User &user)
{
	if (!this->isInChannel(user))
		return ;
	u_int i = 0;
	for (; i<_users.size(); i++)
	{
		if (*(_users[i].second) == user)
			break ;
	}
	user.removeChannl(_name);
	this->_users.erase(this->_users.begin() + i);
}

void			Channel::sendAll(std::string msg, std::string sender) const
{
	for (size_t i = 0; i < _users.size(); i++)
	{
		if (sender == "" || sender != _users[i].second->getNick())
			_server->send_msg(msg, *(_users[i].second));
	}
}

std::string		Channel::getStrUsers() const
{
	std::string s = "";
	for(size_t i= 0;i < _users.size() ; i++)
	{
		if (_users[i].first)
			s += (_users[i].first) + (_users[i].second)->getNick() +" ";
		else
			s += (_users[i].second)->getNick() +" ";
	}
	std::cout <<"USER: " << s << std::endl;
	return s;
}

std::string		Channel::getLastStrUser()
{
	size_t i = _users.size() - 1;
	if ((_users[i].first))
		return (_users[i].first) + (_users[i].second)->getNick();
	return (_users[i].second)->getNick();
}

bool			Channel::isInChannel(User const & user) const
{
	return (isInChannel(user.getNick()));
}

bool			Channel::isInChannel(std::string const & nick) const
{
	for (u_int i=0; i < this->_users.size(); i++)
	{
		if ((_users[i].second)->getNick() == nick)
			return true;
	}
	return false;
}

bool 			Channel::removeUser(std::string const & nick)
{
	if (!this->isInChannel(nick))
		return false;
	u_int i = 0;
	for (; i<_users.size(); i++)
	{
		if ((*(_users[i].second)).getNick() == nick)
			break ;
	}
	(*_users[i].second).removeChannl(_name);
	this->_users.erase(this->_users.begin() + i);
	return (true);
}

bool			Channel::removeUser(User &user)
{
	return (this->removeUser(user.getNick()));
	/*if (!this->isInChannel(user))
		return false;
	u_int i = 0;
	for (; i<_users.size(); i++)
	{
		if (*(_users[i].second) == user)
			break ;
	}
	this->_users.erase(this->_users.begin() + i);
	return (true);*/
}

bool			Channel::isOperator(User &user)
{
	u_int i = 0;
	for( ; i < _users.size(); i++)
	{
		if (_users[i].first == '@' && user == *_users[i].second)
			return true;
	}
	return false;
}

bool			Channel::isOperator(std::string &user)
{
	u_int i = 0;
	for( ; i < _users.size(); i++)
	{
		if ( _users[i].first == '@' && user == _users[i].second->getNick())
			return true;
	}
	return false;
}

void			Channel::kick(User &user, std::list<std::string> & users, std::string msg)
{ 
	if (!this->isInChannel(user))
	{
		_server->getHandler()._numeric_reply(442, user, _name);
		return;
	}
	if (!isOperator(user))
	{
		_server->getHandler()._numeric_reply(482, user, _name);
		return;
	}
	for (std::list<std::string>::iterator i = users.begin(); i != users.end(); i++)
	{
		if (this->isInChannel(*i))
		{
			msg = ":" + user.getNick() + "!" +  user.getUsername() + " KICK "+ _name+ " " + *i +" :" + msg + "\r\n";
			sendAll(msg);
			this->removeUser(*i);
		}
		else
			_server->getHandler()._numeric_reply(441, user, *i + " " + _name);
	}

}

void 				Channel::getTopic(User &user) const
{
	if(_topic == "")
		_server->getHandler()._numeric_reply(331, user, _name);
	else 
	{
		_server->getHandler()._numeric_reply(332, user, _name + " :"+ _topic);
		_server->getHandler()._numeric_reply(333, user, _name + " " + _topicSetter + " " + getTopicTime());
	}
}


void				Channel::setTopic(User &user, std::string &topic)
{
	if (!this->isInChannel(user))
		_server->getHandler()._numeric_reply(442, user, _name);
	else if (!isOperator(user))
		_server->getHandler()._numeric_reply(482, user, _name);
	else 
	{
		_topic = topic;
		_topicSetter = user.getNick();
		_topicTime = std::time(nullptr);
		std::string msg = ":" + user.getNick() + "!" +  user.getUsername() + " TOPIC " + _name + " :"+ _topic + "\n\r";
		_server->send_msg(msg , user);
	}
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

	int					Channel::getLimit() const
	{
		return (_limit);
	}

	std::string const & Channel::getModes() const
	{ return (this->_modes); }

	std::string 	Channel::getName() const
	{return _name;}

	std::string 	Channel::getKey() const
	{
		return(_key);
	}
	std::string 	Channel::getTopic() const
	{
		return(_topic);
	}

	std::string 	Channel::getCreationTime() const
	{
		return(std::to_string(_creationTime));
	}

	std::string 	Channel::getTopicTime() const
	{
		return(std::to_string(_topicTime));
	}

	void 			Channel::setStatus( std::string nick, char status = 0)
	{
		size_t i= 0;
		for( ; i < _users.size(); i++)
		{
			if (nick == (_users[i].second)->getNick())
				break;
		}
		_users[i].first = status;
	}

std::vector<std::pair<char,User *> > const	&Channel::getUserList() const
{
	return (_users);
}

Channel::ban_list_type const			&Channel::getBanList() const
{
	return (_banList);
}
Channel::excIB_list_type const		&Channel::getExeInviteList() const
{
	return (_excInviteList);
}
Channel::excIB_list_type const		&Channel::getExeBanList() const
{
	return (_excBanList);
}
/*
** --------------------------------- EXCEPTION --------------------------------
*/
const char *	Channel::InvalidName::what() const throw()
{
	return ("InvalidName");
}

/* ************************************************************************** */
