#include "Channel.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Channel::Channel() {}

Channel::Channel(std::string name) : _name(name), _key(""), _topic("")/*, _founder(us)*/
{
}

Channel::Channel(std::string name, std::string key): _name(name), _key(key), _topic("")/*, _founder(us)*/
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

/*Channel &				Channel::operator=( Channel const & rhs )
{
	//if ( this != &rhs )
	//{
		//this->_value = rhs.getValue();
	//}
	return *this;
}*/

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
	if (key == _key)
	{
		_users.push_back(std::pair<char,User *>(status, &user));
		return (1);
	}
	return(475);	//ERR_BADCHANNELKEY (475)
}

std::string		Channel::getStrUsers()
{
	std::string s = "";
	for(size_t i= 0 ; i < _users.size(); i++)
		s += _users[i].first  + (_users[i].second)->getNick() + " ";
	return s;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/
	std::string 	Channel::getName() const
	{return _name;}

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
