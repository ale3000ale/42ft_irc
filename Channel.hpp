#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include "User.hpp"

#define LOCAL		'&'

class Server;

class Channel
{
	private:
		std::string 	_name;
		std::string		_key;
		std::string 	_topic;
		Server			*_server;
		//User 			*_founder;
		std::vector<std::pair<char,User *> > _users;

	public:
		Channel();
		Channel(std::string name,Server &server);
		Channel(std::string name, std::string key, Server &server);
		Channel(Channel const & channel);
		~Channel();

		/* Channel &		operator=( Channel const & rhs ); */

		/*----METHODS----*/
		int				join_user(User &user, std::string key , char status);
		void			sendAll(std::string msg);
		std::string		getStrUsers();
		std::string		getLastStrUser();


		/*----GETTER----*/
		std::string 	getName() const;
		std::string 	getKey() const;
		std::string 	getTopic() const;
		
		/*----SETTER----*/
		void 			setStatus( std::string nick, char status);
	
		class InvalidName: public std::exception
		{ 
			const char *what() const throw();
		};
};

//std::ostream &			operator<<( std::ostream & o, Channel const & i );
User 					*join_channel(std::string arg);
#endif /* ********************************************************* CHANNEL_H */
