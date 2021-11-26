
#include "User.hpp"


User::User(int fd) : _socket_fd(fd), _pass_set(false), _registered(false) {}

User::~User() {};

std::string& User::buffer() { return (this->_buffer); }

bool		User::is_passed() const { return(this->_pass_set); }
bool		User::is_registered() const { return(this->_registered); };

void		User::set_passed() { this->_pass_set= true; }
void		User::set_registered() { this->_registered = true; }