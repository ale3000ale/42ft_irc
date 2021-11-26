
#ifndef USER_HPP
# define USER_HPP

#include <string>

class User
{
	public:
		User();
		~User();

		std::string&	buffer();
		void			exec_cmd();

	private:
		bool		_registered;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _buffer;
};


#endif