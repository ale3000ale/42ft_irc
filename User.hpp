
#ifndef USER_HPP
# define USER_HPP

#include <string>

class User
{
	public:
		User(int fd, std::string host);
		~User();

		std::string&	buffer();

		bool		is_passed() const;
		bool		is_registered() const;

		void		set_passed();
		void		set_registered();

		std::string const &	getHost() const;
		int	getSocket() const;
		std::string const &	getNick() const;
		void				setNick(std::string nick);
		std::string const &	getUsername() const;
		void				setUsername(std::string nick);
		//std::string const &	getRealname() const;
		void				setRealname(std::string nick);
		bool				isAway() const;
		void				setAway(bool away, std::string msg = "");
		std::string const & getAwayMsg() const;
	private:
		int			_socket_fd;
		std::string _host;
		bool		_pass_set;
		bool		_registered;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _buffer;
		bool		_away;
		std::string	_away_msg;
};


#endif