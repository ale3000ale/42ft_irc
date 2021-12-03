#include "Channel.hpp"
#include "Server.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Channel::Channel()
{

}

Channel::Channel(std::string name, Server &server) : 
	_name(name), _key(""), _topic(""), _server(&server)
{
}

Channel::Channel(std::string name, std::string key, Server &server): 
	_name(name), _key(key), _topic(""), _server(&server)/*, _founder(us)*/
{
}

Channel::Channel(Channel const & ch): 
	_name(ch._name), _key(ch._key), _topic(ch._topic), _server(ch._server), _users(ch._users)
{
	
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

bool				Channel::empty()
{
	return _users.empty();
}

std::string const & Channel::getModes() const
{ return (this->modes); }

void				Channel::addMode()
{
	//TODO:
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
		_server->getHandler()._numeric_reply(353, user, _name);
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

std::string		Channel::getStrUsers()
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
	/*for (u_int i=0; i < this->_users.size(); i++)
	{
		if (*(_users[i].second) == user)
			return true;
	}
	return false;*/
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
		std::time_t result = std::time(nullptr);
		_server->getHandler()._numeric_reply(332, user, _name + " :"+ _topic);
		_server->getHandler()._numeric_reply(333, user, _name + " " + _topicSetter + " " + std::to_string(result));
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
		std::string msg = ":" + user.getNick() + "!" +  user.getUsername() + " TOPIC " + _name + " :"+ _topic + "\n\r";
		_server->send_msg(msg , user);
		//:pino!ciao@newnet-nrebgj.business.telecomitalia.it TOPIC #ai :puzzi
	}
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/
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
	
/*
** --------------------------------- EXCEPTION --------------------------------
*/
const char *	Channel::InvalidName::what() const throw()
{
	return ("InvalidName");
}

/* ************************************************************************** */
