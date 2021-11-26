
#include "Server.hpp"

#include <iostream>

Server::Server(std::string port, std::string password) : _port(port), _password(password), _handler(password)
{
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(nullptr, this->_port.c_str(), &hints, &ai)))
		throw std::runtime_error(gai_strerror(rv));
    
	if ((this->_socket_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0)
		throw std::runtime_error(strerror(errno));

	// Lose the pesky "address already in use" error message
    setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if (bind(this->_socket_fd, ai->ai_addr, ai->ai_addrlen) < 0)
	{
		close(this->_socket_fd); // da capire se devo fare la delete nel catch o meno
		freeaddrinfo(ai);
		throw std::runtime_error(strerror(errno));
	}

	freeaddrinfo(ai); // All done with this
}

Server::~Server()
{
	close(this->_socket_fd);
}

void Server::run()
{
	if ((listen(this->_socket_fd, BACKLOG)) < 0)
		throw std::runtime_error(strerror(errno));
	
	// Add server fd to set
	_addFd(this->_socket_fd);

	while (1)
	{
		if (poll(this->_pfds.data(), this->_pfds.size(), -1) < 0)
			throw std::runtime_error(strerror(errno));
		for(u_int i = 0; i < this->_pfds.size(); i++)
		{
			// Check if someone's ready to read
			if (this->_pfds[i].revents & POLLIN)
			{
				if (this->_pfds[i].fd == this->_socket_fd)
				{
					// If listener is ready to read, handle new connection
					_addUser();
				}
				else
				{
					// If not the listener, we're just a regular client
					char buf[512];
					memset(buf, 0, sizeof(buf));
					int nbytes = recv(this->_pfds[i].fd, buf, sizeof(buf), 0);
					if (nbytes <= 0)
					{
						/*if (nbytes)
							ERROR*/
						// Got error or connection closed by client
						_deleteUser(i); // check if doing this i miss one fd
					}
					else
					{
						User &curr = this->_users[i - 1];
						curr.buffer() += buf;
						if (curr.buffer().find("\r\n") != std::string::npos)
							_exec_cmd(curr);
					}
				}
			}
		}
	}
}

void Server::_addUser()
{
	struct sockaddr_storage clientaddr; // Client address
	socklen_t addrlen = sizeof(clientaddr);
	int new_fd;
	if ((new_fd = accept(this->_socket_fd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
		return (perror("accept"));
	_addFd(new_fd);
	this->_users.push_back(User(new_fd));
}

void Server::_deleteUser(int index)
{
	close(this->_pfds[index].fd); // closing client's fd
	this->_pfds.erase(this->_pfds.begin() + index); 
	this->_users.erase(this->_users.begin() + index - 1);
}

void Server::_addFd(int new_fd)
{
	struct pollfd tmp;

	fcntl(new_fd, F_SETFL, O_NONBLOCK);
	tmp.fd = new_fd;
	tmp.events = POLLIN; // Report ready to read on incoming connection
	this->_pfds.push_back(tmp);
}

bool Server::checkPass(std::string& pass)
{
	return (pass == this->_password);
}

void Server::_exec_cmd(User& executor)
{
	this->_handler.handle(executor.buffer(), executor);
	executor.buffer().clear();
}

bool	Server::exist_channel(std::string name)
{
	if(_channels.find(name) != _channels.end())
		return true;
	else
		return false;
}

Channel			&Server::get_channel(std::string name)
{
	return _channels[name];
}

bool			Server::add_channel(Channel ch)
{
	if(_channels.find(ch.getName()) == _channels.end())
	{
		_channels[ch.getName()] = ch;
		return true;
	}
	return false;
}