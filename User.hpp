
#ifndef USER_HPP
# define USER_HPP

#include <string>
#include "Command.hpp"

class User
{
	public:
		User(int fd, Server& server);
		~User();

		std::string&	buffer();
		void			exec_cmd();

		bool		is_passed() const;
		bool		is_registered() const;

		void		set_passed();
		void		set_registered();
	private:
		Server&		_server;
		int			_socket_fd;
		bool		_pass_set;
		bool		_registered;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _buffer;
};


#endif