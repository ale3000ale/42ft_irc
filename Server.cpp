
#include "Server.hpp"
#include "Channel.hpp"

#include <iostream>

Server::Server(std::string port, std::string password) : _port(port), _password(password), _handler(*this)
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
						//std::cout<<curr.buffer()<<" size:"<<curr.buffer().size()<<"\n";
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
	char remoteIP[INET6_ADDRSTRLEN];
	struct sockaddr *casted_addr = (struct sockaddr*)&clientaddr;
	if (casted_addr->sa_family == AF_INET)
    	inet_ntop(AF_INET, &(((struct sockaddr_in*)casted_addr)->sin_addr), remoteIP, INET_ADDRSTRLEN);
    else
		inet_ntop(AF_INET6, &(((struct sockaddr_in6*)casted_addr)->sin6_addr), remoteIP, INET6_ADDRSTRLEN);
	this->_users.push_back(User(new_fd, remoteIP));

}

void Server::_deleteUser(int index)
{
	close(this->_pfds[index].fd); // closing client's fd
	this->_pfds.erase(this->_pfds.begin() + index); 
	this->_users.erase(this->_users.begin() + index - 1);
}

void Server::deleteUser(std::string nick)
{
	for (u_int i = 0; i < this->_users.size(); i++)
	{
		if (this->_users[i].getNick() == nick)
			return (_deleteUser(i));
	}
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

std::vector<User> const & Server::getUserList() const
{
	return (this->_users);
}

void Server::_exec_cmd(User& executor)
{
	std::string& buffer = executor.buffer();
	//std::cout <<"1: "<<executor.buffer()<<"\n";
	int pos = buffer.find("\r\n");
	do
	{
		//std::cout <<"2: "<<buffer.substr(0, pos)<<"\n";
		this->_handler.handle(buffer.substr(0, pos), executor);
		buffer.erase(0, pos + 2);
		//std::cout <<"3: "<<buffer<<"\n";
		pos = buffer.find("\r\n");
	} while (pos != -1);
	//std::cout <<"4: "<<executor.buffer()<<"\n";
	//executor.buffer().clear();
}

bool	Server::exist_channel(std::string name) const
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
		_channels.insert ( std::pair<std::string ,Channel>(ch.getName(), ch) );
		return true;
	}
	return false;
}

void	Server::send_msg(std::string& msg, User const & target) const
{
	if (send(target.getSocket(), msg.c_str(), msg.length(), 0) < 0)
        perror("send");
}

int		Server::send_msg(std::string& msg, std::string target, User const & owner)
{
	if (exist_channel(target))
	{
		Channel& tmp_chan = get_channel(target);
		if (!tmp_chan.isInChannel(owner))
			return (404);
		else
			tmp_chan.sendAll(msg, owner.getNick());
	}
	else
		return (401);
	return (0);
}

int		Server::send_msg(std::string& msg, std::string target) const
{
	u_int i = 0;
	
	while (i < _users.size())
	{
		if (_users[i].getNick() == target)
		{
			if (_users[i].isAway())
				return (301);
			send_msg(msg, _users[i]);
			break ;
		}
		i++;
	}
	if (i == _users.size())
		return (401);
	return 0;
}

CommandHandler	Server::getHandler() const
{
	return (_handler);
}

void			Server::sendAllChans(std::string msg, User& sender)
{
	_chan_it it = this->_channels.cbegin();
	while (it != this->_channels.cend())
	{
		send_msg(msg, (*it).first, sender);
		++it;
	}
}

User const 		&Server::getUser(std::string user) const
{
	size_t i  = 0;
	for (; i < _users.size(); i++)
	{
		if (_users[i].getUsername() == user)
			return _users[i];
	}
	return _users[i];
}
