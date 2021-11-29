#include "Channel.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Channel::Channel() {}

Channel::Channel(std::string name, User &us) : _name(name), _key(""), _topic("")/*, _founder(us)*/
{
	_users.push_back(&us);
}

Channel::Channel(std::string name, std::string key, User &us): _name(name), _key(key), _topic("")/*, _founder(us)*/
{
	_users.push_back(&us);
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

int			Channel::join_user(User &user, std::string key , char status)
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
	for(int i= 0 ; i < _users.size(); i++)
		s += _users[i].first  + *(_users[i].second)->getNick + ! + 
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/
	std::string 	Channel::getName() const
	{return _name;}
/*
** --------------------------------- EXCEPTION --------------------------------
*/
const char *	Channel::InvalidName::what() const throw()
{
	return ("InvalidName");
}

/* ************************************************************************** */
