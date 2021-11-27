
#ifndef USER_HPP
# define USER_HPP

#include <string>
#include "CommandHandler.hpp"

class User
{
	public:
		User(int fd);
		~User();

		std::string&	buffer();

		bool		is_passed() const;
		bool		is_registered() const;

		void		set_passed();
		void		set_registered();

		std::string const &	getNick() const;
		void				setNick(std::string nick);
		std::string const &	getUsername() const;
		void				setUsername(std::string nick);
		//std::string const &	getRealname() const;
		void				setRealname(std::string nick);
	private:
		int			_socket_fd;
		bool		_pass_set;
		bool		_registered;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _buffer;
};


#endif