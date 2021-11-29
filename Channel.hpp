#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include "User.hpp"

#define LOCAL		'&'

class Channel
{
	private:
		std::string 	_name;
		std::string		_key;
		std::string 	_topic;
		//User 			*_founder;
		std::vector<std::pair<char,User *> > _users;

	public:
		Channel();
		Channel(std::string name);
		Channel(std::string name, std::string key);
		~Channel();

		//Channel &		operator=( Channel const & rhs );

		int				join_user(User &user, std::string key , char status);

		std::string		getStrUsers();
		std::string 	getName() const;
		void 			setStatus( std::string nick, char status);
	
		class InvalidName: public std::exception
		{ 
			const char *what() const throw();
		};
};

//std::ostream &			operator<<( std::ostream & o, Channel const & i );
User 					*join_channel(std::string arg);
#endif /* ********************************************************* CHANNEL_H */
