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

int			Channel::join_user(User &user, std::string key , char status = 0)
{
	if (this->isInChannel(user))
	{
		std::cout << "User: " + user.getNick() + ":" + user.getUsername() + " is already in the channel!\n";
		return (0);
	}
	if (key == _key)
	{
		_users.push_back(std::pair<char,User *>(status, &user));
		std::string msg = ":" + user.getNick() + "!" +  user.getUsername() + '@' + user.getHost() + " JOIN :" + _name + "\r\n";
		this->sendAll(msg);
		//_server->getHendler()._numeric_reply(332, user, _name); ONLY TO DO IF TOPIC IS SET
		_server->getHandler()._numeric_reply(353, user, _name);
		_server->getHandler()._numeric_reply(366, user, _name);
		return (1);
	}
	return(475);	//ERR_BADCHANNELKEY (475)
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
	for (u_int i=0; i < this->_users.size(); i++)
	{
		if (*(_users[i].second) == user)
			return true;
	}
	return false;
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


	
/*
** --------------------------------- EXCEPTION --------------------------------
*/
const char *	Channel::InvalidName::what() const throw()
{
	return ("InvalidName");
}

/* ************************************************************************** */
