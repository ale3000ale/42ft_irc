#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include "User.hpp"
#include <list>

#define LOCAL		'&'

class Server;

class Channel
{
	public:
		typedef  std::vector<std::pair<char,User *> > user_list_type;
	private:
		std::string 	_name;
		std::string		_key;
		std::string 	_topic;
		std::string 	_topicSetter;
		Server			*_server;
		//User 			*_founder;
		std::string		modes;
		std::vector<std::pair<char,User *> > _users;

	public:
		Channel();
		Channel(std::string name,Server &server);
		Channel(std::string name, std::string key, Server &server);
		Channel(Channel const & channel);
		~Channel();

		/* Channel &		operator=( Channel const & rhs ); */

		/*----METHODS----*/
		int					join_user(User &user, std::string key , char status);
		bool				removeUser(User &user);
		bool 				removeUser(std::string const & nick);
		void				part_user(User &user);
		void				sendAll(std::string msg, std::string sender = "") const;
		std::string			getStrUsers();
		std::string			getLastStrUser();
		std::vector<std::pair<char,User *> > const	&getUserList() const;
		bool				isInChannel(User const & user) const;
		bool				isInChannel(std::string const & nick) const;
		std::string const & getModes() const;
		void				addMode();
		void				kick(User &user, std::list<std::string> &users, std::string msg = "Because me stai sul cazzo");
		bool				isOperator(User &user);
		bool				isOperator(std::string &user);
		bool				empty();
		/*----GETTER----*/
		std::string 		getName() const;
		std::string 		getKey() const;
		std::string 		getTopic() const;
		void 				getTopic(User &user) const;
		void				setTopic(User &user, std::string &topic);
		
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
