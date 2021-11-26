
#include "User.hpp"

#include <iostream>

User::User() : _registered(false) {}

User::~User() {};

std::string& User::buffer() { return (this->_buffer); }

void User::exec_cmd()
{
	if (!this->_registered)
	 std::cout<<"NOT REGISTERED\n";

}