
#include "Bot.hpp"

Bot::Bot(std::string server_ip, std::string port, std::string password) : _server_ip(server_ip), _port(port), _password(password)
{
	struct addrinfo hints, *ai;
	int rv, yes=1;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(server_ip.c_str(), port.c_str(), &hints, &ai)) != 0)
		throw std::runtime_error("getaddrinfo: " + std::string(gai_strerror(rv)));
	if ((this->_socket_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0)
		throw std::runtime_error("socket: " + std::string(strerror(errno)));
	setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (connect(this->_socket_fd, ai->ai_addr, ai->ai_addrlen) == -1)
	{
		close(this->_socket_fd);
		throw std::runtime_error("connect: " + std::string(strerror(errno)));
	}
	freeaddrinfo(ai);
}

Bot::~Bot() {}

void	Bot::run()
{
	int rv = _register();
	if (rv != 1)
	{
		close(this->_socket_fd);
		if (!rv)
			throw std::runtime_error("Connection to server lost");
		if (rv < 0)
			throw std::runtime_error("recv: " + std::string(strerror(errno)));
		if (rv == 464)
			throw std::runtime_error("Incorrect pasword");
	}
	else
		std::cout<<"Bot correctly connected!"<<std::endl;
	while (1)
	{
		memset(this->_buff, 0, sizeof(this->_buff));
		int nbytes = recv(this->_socket_fd, this->_buff, sizeof(this->_buff), 0);
		if (!nbytes) // lost connection to server
			throw std::runtime_error("Connection to server lost");
		if (nbytes < 0)
			throw std::runtime_error("recv: " + std::string(strerror(errno)));
		// TODO: parse messages
		std::cout<<this->_buff;
	}
}

int	Bot::_register()
{
	std::string msg;
	int nbytes;
	sleep(1);
	msg = "PASS " + this->_password + "\r\n";
	send(this->_socket_fd, msg.c_str(), msg.length(), 0);
	msg = "NICK insultaBOT\r\n";
	send(this->_socket_fd, msg.c_str(), msg.length(), 0);
	msg = "USER bot_user 0 * :insulta BOT\r\n";
	send(this->_socket_fd, msg.c_str(), msg.length(), 0);
	msg = "JOIN #insultaBOT\r\n";
	send(this->_socket_fd, msg.c_str(), msg.length(), 0);
	memset(this->_buff, 0, sizeof(this->_buff));
	nbytes = recv(this->_socket_fd, this->_buff, sizeof(this->_buff), 0);
	
	if (nbytes <= 0)
		return (nbytes);
	int numeric = _get_numeric(this->_buff);
	return (numeric);
}

int Bot::_get_numeric(std::string cmd) const
{
	cmd.erase(0, cmd.find(" ") + 1);
	std::string numeric = cmd.substr(0, cmd.find(" "));
	return (std::atoi(numeric.c_str()));
}