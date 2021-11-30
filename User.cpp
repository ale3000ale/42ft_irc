
#include "User.hpp"


User::User(int fd, std::string host) : _socket_fd(fd), _host(host), _pass_set(false), _registered(false), _away(false) {}

User::~User() {};

std::string& User::buffer() { return (this->_buffer); }

bool		User::is_passed() const { return(this->_pass_set); }
bool		User::is_registered() const { return(this->_registered); };

void		User::set_passed() { this->_pass_set= true; }
void		User::set_registered() { this->_registered = true; }

std::string const &	User::getHost() const
{ return (this->_host); }

int	User::getSocket() const
{ return (this->_socket_fd); };

std::string const &	User::getNick() const
{ return (this->_nickname); }

void				User::setNick(std::string nick)
{ this->_nickname = nick; }

std::string const &	User::getUsername() const
{ return (this->_username); }

void				User::setUsername(std::string username)
{ this->_username = username; }

void				User::setRealname(std::string realname)
{ this->_realname = realname; }

bool				User::isAway() const
{ return (this->_away); }

void				User::setAway(bool away, std::string msg)
{
	this->_away = away;
	if (away)
		this->_away_msg = msg;
	else
		this->_away_msg.clear();
}

std::string const & User::getAwayMsg() const
{ return (this->_away_msg); }