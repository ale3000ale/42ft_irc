
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

void	parse_args(int ac, char **av, std::string& server_ip, std::string& port, std::string& password)
{
	int i = 0;
	if (ac == 4)
		server_ip = av[i++];
	port = av[i++];
	password = av[i];
}

int get_numeric(std::string cmd)
{
	cmd.erase(0, cmd.find(" ") + 1);
	std::string numeric = cmd.substr(0, cmd.find(" "));
	return (std::atoi(numeric.c_str()));
}

int main(int ac, char *av[])
{
	std::cout<<ac<<std::endl;
	// [server_ip] port password
	if (ac != 3 && ac != 4)
	{
		std::cerr<<"usage: "<<av[0]<<" [server_ip] port password"<<std::endl;
		return (1);
	}
	std::string server_ip = "127.0.0.1";
	std::string port;
	std::string password;
	parse_args(ac, &av[1], server_ip, port, password);

	// connection to server
	struct addrinfo hints, *ai;
	int rv, sockfd, yes=1;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(server_ip.c_str(), port.c_str(), &hints, &ai)) != 0)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
		return 1;
	}
	if ((sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0)
	{
		perror("client: socket");
		return 1;
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) == -1)
	{
		perror("client: connect");
		close(sockfd);
		return 1;
	}
	freeaddrinfo(ai);

	std::string msg;
	/*password = "123";*/
	msg = "PASS " + password + "\r\n";
	send(sockfd, msg.c_str(), msg.length(), 0);
	msg = "NICK insultaBOT\r\n";
	send(sockfd, msg.c_str(), msg.length(), 0);
	msg = "USER bot_user 0 * :insulta BOT\r\n";
	send(sockfd, msg.c_str(), msg.length(), 0);
	char buf[512];
	memset(buf, 0, sizeof(buf));
	/*int nbytes = */recv(sockfd, buf, sizeof(buf), 0);
	if (get_numeric(buf) == 464)
	{
		std::cout<<"Error: incorrect password"<<std::endl;
		close(sockfd);
		return 1;
	}
	msg = "JOIN #insultaBOT\r\n";
	send(sockfd, msg.c_str(), msg.length(), 0);
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		int nbytes = recv(sockfd, buf, sizeof(buf), 0);
		//if (nbytes == 0)
			// TODO: server closed connection, try to reconnect
	}
}