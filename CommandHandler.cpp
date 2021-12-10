
#include "CommandHandler.hpp"
#include "Server.hpp"

CommandHandler::CommandHandler(Server	&server): _server(server)
{
	this->_handlers["PASS"] = &CommandHandler::_handlePASS;
	this->_handlers["NICK"] = &CommandHandler::_handleNICK;
	this->_handlers["USER"] = &CommandHandler::_handleUSER;
	this->_handlers["MOTD"] = &CommandHandler::_handleMOTD;
	this->_handlers["LUSERS"] = &CommandHandler::_handleLUSERS;
	this->_handlers["PING"] = &CommandHandler::_handlePING;
	this->_handlers["JOIN"] = &CommandHandler::_handleJOIN;
	this->_handlers["PART"] = &CommandHandler::_handlePART;
	this->_handlers["PRIVMSG"] = &CommandHandler::_handlePRIVMSG;
	this->_handlers["AWAY"] = &CommandHandler::_handleAWAY;
	this->_handlers["QUIT"] = &CommandHandler::_handleQUIT;
	this->_handlers["WHO"] = &CommandHandler::_handleWHO;
	this->_handlers["KICK"] = &CommandHandler::_handleKICK;
	this->_handlers["MODE"] = &CommandHandler::_handleMODE;
	this->_handlers["TOPIC"] = &CommandHandler::_handleTOPIC;
	this->_handlers["NAMES"] = &CommandHandler::_handleNAMES;
	this->_handlers["INVITE"] = &CommandHandler::_handleINVITE;
	this->_handlers["LIST"] = &CommandHandler::_handleLIST;
}

void CommandHandler::_parse_cmd(std::string cmd_line)
{
	if (cmd_line.empty())
		return ;
	int pos = cmd_line.find(" ");
	this->_command = toUpper(cmd_line.substr(0, pos));
	cmd_line.erase(0, (pos != -1) ? pos + 1 : pos);
	this->_params.clear();

	while (!cmd_line.empty())
	{
		if (cmd_line[0] == ':')
		{
			cmd_line.erase(0, 1);
			if (cmd_line.empty())
			{
				this->_params.push_back("");
				break ;
			}
			this->_params.push_back(cmd_line);
			cmd_line.erase(0);
		}
		else
		{
			pos = cmd_line.find(" ");
			this->_params.push_back(cmd_line.substr(0, pos));
			cmd_line.erase(0, (pos != -1) ? pos + 1 : pos);
		}
	}
}

void CommandHandler::handle(std::string cmd_line, User& owner)
{
	std::cout<<"command: "<<cmd_line<<std::endl;
	_parse_cmd(cmd_line);
	if (this->_command.empty())
		return ;
	if (!owner.is_passed() && this->_command != "PASS")
		return ;
	else if (owner.is_passed() && !owner.is_registered() && this->_command != "NICK" && this->_command != "USER")
		return numeric_reply(451, owner, this->_command); // ERR_NOTREGISTERED
	if (this->_handlers.find(this->_command) == this->_handlers.end())
		numeric_reply(421, owner, this->_command); // ERR_UNKNOWNCOMMAND
	else
		(*this.*(this->_handlers[this->_command]))(owner);
}

void CommandHandler::_handlePASS(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS
	if (owner.is_registered())
		return numeric_reply(462, owner); //ERR_ALREADYREGISTERED
	if (this->_server.checkPass(this->_params.front()))
		owner.set_passed();
	else
		numeric_reply(464, owner); // ERR_PASSWDMISMATCH
}

void CommandHandler::_handleNICK(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return numeric_reply(431, owner); // ERR_NONICKNAMEGIVEN
	std::string& nick = this->_params.front();
	std::vector<User*> const & users = this->_server.getUserList();
	for (u_int i = 0; i < users.size(); i++)
	{
		if (*users[i] == nick)
			return numeric_reply(433, owner, nick); ; // ERR_NICKNAMEINUS
	}
	std::string old_nick = owner.getNick();
	owner.setNick(nick);
	if (old_nick != "")
	{
		std::string msg = ":" + old_nick + "!" + owner.getUsername() + "@" + owner.getHost() + " NICK :" + owner.getNick() + "\r\n";
		this->_server.send_msg(msg, owner);
	}
	if (!owner.is_registered() && !owner.getUsername().empty())
		_welcome_msg(owner);
}

void CommandHandler::_handleUSER(User& owner) 
{
	if (this->_params.size() != 4)
		return numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS
	if (owner.is_registered())
		return numeric_reply(462, owner); //ERR_ALREADYREGISTERED
	std::string username = this->_params.front();
	if (username.empty()) // i think this case wont ever occur
		return numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS (461)
	std::string realname = this->_params.back();
	owner.setUsername(username);
	owner.setRealname(realname);
	if (!owner.getNick().empty())
		_welcome_msg(owner);
}

void CommandHandler::_handleMOTD(User& owner)
{
	if (this->_command == "MOTD" && this->_params.size() && this->_params.front() != "myIRCServer")
		return numeric_reply(402, owner, this->_params.front()); // ERR_NOSUCHSERVER
	std::vector<std::string> motd = this->_server.getMotd();
	if (!motd.size())
		return numeric_reply(422, owner); // ERR_NOMOTD
	numeric_reply(375, owner); // RPL_MOTDSTART
	for (u_int i=0; i < motd.size(); i++)
		numeric_reply(372, owner, motd[i]); // RPL_MOTD
	numeric_reply(376, owner); // RPL_ENDOFMOTD
}

void CommandHandler::_handleLUSERS(User& owner)
{
	int user_count = this->_server.getUserList().size();
	int chan_count = this->_server.getchannelList().size();
	numeric_reply(251, owner, std::to_string(user_count)); // RPL_LUSERCLIENT
	numeric_reply(252, owner); // RPL_LUSEROP
	numeric_reply(254, owner, std::to_string(chan_count)); // RPL_LUSERCHANNELS
	numeric_reply(255, owner, std::to_string(user_count)); // RPL_LUSERME
}

void CommandHandler::_handlePING(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS
	std::string msg = ":myIRCServer PONG myIRCServer :" + this->_params.front() + "\r\n";
	this->_server.send_msg(msg, owner);
}

void CommandHandler::_handlePRIVMSG(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return numeric_reply(411, owner, this->_command); // ERR_NORECIPIENT
	if (this->_params.size() == 1)
		return numeric_reply(412, owner); // ERR_NOTEXTTOSEND

	std::string targets = this->_params.front();
	_iterator it = ++this->_params.begin();
	std::string text = " :" + *it;
	for (++it; it != this->_params.cend(); ++it)
		text += " "+*it;
	std::string head = ":" + owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost() + " PRIVMSG ";
	while (!targets.empty())
	{
		int pos = targets.find(",");
		std::string curr_target = targets.substr(0, pos);
		std::string msg = head + curr_target + text + "\r\n";
		int rv;
		if (curr_target[0] == '#')
			rv = this->_server.send_msg(msg, curr_target, owner);
		else
			rv = this->_server.send_msg(msg, curr_target);
		if (rv)
			numeric_reply(rv, owner, curr_target);
		targets.erase(0, (pos != -1) ? pos + 1 : pos);
	}
}

void CommandHandler::_handleAWAY(User& owner)
{
	if (!this->_params.size())
	{
		owner.setAway(false);
		numeric_reply(305, owner); // RPL_UNAWAY
	}
	else
	{
		owner.setAway(true, this->_params.front());
		numeric_reply(306, owner); // RPL_NOWAWAY
	}
}

void CommandHandler::_handleJOIN(User& owner)
{
	if (_params.empty())
		return ; //ERR_NEEDMOREPARAMS (461)
	std::list<std::string> names;
	std::list<std::string> keys;
	int pos;
	
	while( _params.front() != "")
	{
		pos = _params.front().find(",");
		names.push_back(_params.front().substr(0, pos));
		_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
	}
	
	_params.pop_front();
	if (!_params.empty())
	{
		while( _params.front() != "")
			{
				pos = _params.front().find(",");
				keys.push_back(_params.front().substr(0, pos));
				_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
			}
		_params.pop_front();
	}
	if (names.front()[0] != '#')
		return (numeric_reply(403,owner, names.front()));
	while(!names.empty())
	{
		char stat  = 0;
		if (!_server.exist_channel(names.front()))
		{
			Channel ch(names.front(), keys.front(), _server);
			_server.add_channel(ch);
			stat = '@';
		}
		Channel &chan = _server.get_channel(names.front());
		std::cout << "JOINING "<< chan.getName() << " key " << chan.getKey() << std::endl;
		chan.join_user(owner, keys.front(), stat);
		if (!keys.empty())
			keys.pop_front();
		names.pop_front();
	}
}

void CommandHandler::_handlePART(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return (numeric_reply(461, owner, this->_command)); // ERR_NEEDMOREPARAMS
	std::string targets = this->_params.front();
	std::string reason;
	if (this->_params.size() > 1 &&  this->_params.front() != "")
	{
		_iterator it = ++this->_params.begin();
		reason = " :\"" + *it;
		for (++it; it != this->_params.cend(); ++it)
			reason += " "+*it;
		reason += "\"";
	}
	std::string head = ":" + owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost() + " PART ";
	while (!targets.empty())
	{
		int pos = targets.find(",");
		std::string curr_target = targets.substr(0, pos);
		std::string msg = head + curr_target + reason + "\r\n";

		if (!this->_server.exist_channel(curr_target))
			numeric_reply(403, owner, curr_target); // ERR_NOSUCHCHANNEL
		else
		{
			Channel& tmp_chan = this->_server.get_channel(curr_target);
			if (!tmp_chan.isInChannel(owner))
				numeric_reply(442, owner, curr_target); // ERR_NOTONCHANNEL
			else
			{
				// send to owner and to other inside channel
				this->_server.send_msg(msg, owner);
				this->_server.send_msg(msg, curr_target, owner);
				tmp_chan.part_user(owner);
				if (tmp_chan.empty())
					_server.removeChannel(tmp_chan.getName());
			}
		}
		targets.erase(0, (pos != -1) ? pos + 1 : pos);
	}
}

void CommandHandler::_handleQUIT(User& owner)
{
	std::string reason = (_params.size() == 1) ? _params.front() : owner.getNick();
	std::string msg = "ERROR :Closing Link: " + owner.getNick() + "[" + owner.getHost() + "] (Quit: " + reason + ")\r\n";
	this->_server.send_msg(msg, owner);
	msg = ":" + owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost() + " QUIT :Quit: " + reason + "\r\n";
	this->_server.sendAllChans(msg, owner);
	std::cout<<"HANDLE QUIT :\n";
	
	this->_server.deleteUser(owner.getNick());
}

void CommandHandler::_handleWHO(User& owner)
{
	const std::vector<User*> &us =  _server.getUserList();
	std::string msg;
	Channel ch;
	if (_params.empty())
	{
		for(size_t i =0 ; i !=us.size(); i++)
		{
			if (!((us[i]->commonChannel(owner.getChannels())) || us[i]->hasMode('i')) || *us[i] == owner)
			{														
				msg = (us[i]->getChannels().empty() ? "* " : us[i]->getChannels().back()+ " ") + us[i]->getUsername() + " " + us[i]->getHost() + " myIRCServer " + us[i]->getNick() +
				 " H :0 "  + us[i]->getRealname();
				numeric_reply(352, owner, msg);
			}
		}
		numeric_reply(315, owner, "*");
	}
	else if(_server.exist_channel(_params.front()))
	{
		ch = _server.get_channel(_params.front());
		const Channel::user_list_type &users = ch.getUserList();
		std::string header = ch.getName(true) + " ";
		for (size_t i =0 ;i != users.size(); i++)
		{
			// TODO: alex vedi un po'se va bene
			msg = header + users[i].second->getUsername() + " " +  users[i].second->getHost() + " myIRCServer " + users[i].second->getNick() + " H";
			if (users[i].first)
				msg += users[i].first;
			msg += " :0 " + users[i].second->getRealname();
			/*msg = header + users[i].second->getUsername() + " " +  users[i].second->getHost() + " myIRCServer " + users[i].second->getNick() +
				" H"; + users[i].first + " :0 " + users[i].second->getRealname();
			else
				msg = ch.getName(true) + " " + users[i].second->getUsername() + " " +  users[i].second->getHost() + " myIRCServer " + users[i].second->getNick() +
				" H :0 " + users[i].second->getRealname();*/
			numeric_reply(352, owner, msg);
		}
		numeric_reply(315, owner, ch.getName(true));
	}
}

void	CommandHandler::_handleKICK(User &owner)
{
	if (_params.size() < 2)
	{
		numeric_reply(461, owner, "KICK");
		return;
	}
	std::list<std::string> channels;
	std::list<std::string> users;
	int pos;
	
	while( _params.front() != "")
	{
		pos = _params.front().find(",");
		channels.push_back(_params.front().substr(0, pos));
		_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
	}
	_params.pop_front();


	while( _params.front() != "")
	{
		pos = _params.front().find(",");
		users.push_back(_params.front().substr(0, pos));
		_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
	}
	_params.pop_front();

	while (!channels.empty())
	{
		if (_server.exist_channel(channels.front()))
		{
			Channel &chan = _server.get_channel(channels.front());
			if (!_params.empty())
				chan.kick(owner,users, _params.front());
			else
				chan.kick(owner,users);
			if (chan.empty())
				_server.removeChannel(chan.getName());
			channels.pop_front();
		}
		else
			numeric_reply(403, owner, channels.front());
	}
}


void	CommandHandler::_handleMODE(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return (numeric_reply(461, owner, this->_command)); // ERR_NEEDMOREPARAMS
	std::string target = this->_params.front();
	if (target[0] == '#') // CHANNEL MODE
	{
		if (!_server.exist_channel(target))
			return (numeric_reply(403, owner, target)); // ERR_NOSUCHCHANNEL
		Channel &ch = _server.get_channel(target);
		if (this->_params.size() == 1 )
		{
			std::cout << "NO MODES\n";
			numeric_reply(324, owner, target + " " + _server.get_channel(target).getModes()); // RPL_CHANNELMODEIS
			numeric_reply(329, owner, target + " " + _server.get_channel(target).getCreationTime()); // RPL_CHANNELMODEIS
			return ;
		}
		_params.pop_front();
		std::string mode = _params.front();
		_params.pop_front();
		char type = (mode[0] == '-' || mode[0] == '+') ? mode[0] : 0;
		std::cout << "NEW MODE |" + mode + "| " <<  type << " TYPE != 0 "<<(type != 0)<<" \n";
		for (size_t i = (type != 0); i < mode.size(); i++)
		{
			std::cout << "MODE " + std::string(1,mode[i]) + " PARAMS "+ _params.front() + "\n";
			if (ch.addMode(owner, mode[i], type, _params.front()))
				_params.pop_front();
		}
		std::cout << "OPERATORE\n";
		
		
	}
	else	// USER MODE
	{
		std::vector<User*> users = this->_server.getUserList();
		uint i=0;
		for (; i<users.size() && users[i]->getNick() != target; i++) ;
		if (i == users.size())
			return (numeric_reply(401, owner, target)); // ERR_NOSUCHNICK
		if (owner.getNick() != target)
			return (numeric_reply(502, owner)); // ERR_USERSDONTMATCH
		if (this->_params.size() == 1 )
			return (numeric_reply(221, owner, target)); // RPL_UMODEIS
		std::string modestring = *(++(this->_params.begin()));
		std::string msg = " ";
		for (i=0; i<modestring.length(); i++)
		{
			char mode = modestring[i];
			if (mode == '+' || mode == '-')
				continue;
			if (UMODES.find(mode) == std::string::npos)
				numeric_reply(501, owner); // ERR_UMODEUNKNOWNFLAG
			else if (i && modestring[i - 1] == '-')
			{
				owner.delMode(mode);
				msg += "-";
				msg += mode;
			}
			else
			{
				owner.addMode(mode);
				msg += "+";
				msg += mode;
			}
		}
		if (msg != " ")
		{
			msg = ":" + owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost() + " MODE " + owner.getNick() + msg + "\r\n";
			this->_server.send_msg(msg, owner);
		}
	}
}

void	CommandHandler::_handleTOPIC(User& owner)
{
	if (_params.size() < 1 || this->_params.front() == "")
		numeric_reply(461, owner, "TOPIC");
	else if (!_server.exist_channel(_params.front()))
		numeric_reply(403, owner, _params.front());
	else 
	{
		Channel &ch = _server.get_channel(_params.front());
		if (_params.size() == 1)
		{
			ch.getTopic(owner);
			
			return;
		}
		_params.pop_front();
		ch.setTopic(owner, _params.front());
	}
}

void	CommandHandler::_handleLIST(User& owner)
{
	numeric_reply(321,owner);
	std::string msg = "";
	const Server::chan_type		&chs = _server.getchannelList();
	for (Server::chan_it i = chs.cbegin(); i != chs.cend(); i++)
	{
		msg = (*i).second.getName(true) + " " + std::to_string((*i).second.getUserCount()) + " :[+" + (*i).second.getModes() + "] " + (*i).second.getTopic();
		numeric_reply(322, owner, msg);
	}
	numeric_reply(323, owner);
}

void	CommandHandler::_handleNAMES(User& owner)
{
	if (_params.size() < 1 || this->_params.front() == "")
	{
		for (std::map<std::string, Channel>::const_iterator i = _server.getchannelList().cbegin();
			i != _server.getchannelList().cend() ; i++)
		{
			std::string msg = "= " + (*i).second.getName(true) + " :"+  (*i).second.getStrUsers() ;
			numeric_reply(353,owner, msg);
			numeric_reply(366,owner, (*i).second.getName(true));
		}
		const std::vector<User*> & users = _server.getUserList();
		std::string msg = ":" + owner.getNick() + "!" +  owner.getUsername() + " ";
		for (size_t i = 0; i < users.size() ; i++)
		{
			if (!users[i]->getChannels().size())
			{
				msg +=  users[i]->getNick() + " * " ;
			}
		}
		msg += "\r\n";
		_server.send_msg( msg, owner);
	}
}	

void		CommandHandler::_handleINVITE(User& owner) 
{
	std::string msg = "";
	std::string nick;
	if (_params.size() < 2 || _params.back() == "")
		return (numeric_reply(461, owner, _command));
	nick = _params.front();
	if (!_server.exist_user(_params.front()))
		return (numeric_reply(401, owner, _params.front()));
	_params.pop_front();
	if (!_server.exist_channel(_params.front()))
		return (numeric_reply(403, owner, _params.front()));
	Channel &ch = _server.get_channel(_params.front());
	ch.invite(owner, nick);
}

void		CommandHandler::numeric_reply(int val, User const &owner, std::string extra) const
{
	std::string msg = ":myIRCServer ";
	if (val < 10)
	{
		msg+= "00";
		msg+= val+'0';
	}
	else
		msg+=std::to_string(val);

	msg += " " + owner.getNick() + " ";
	switch (val)
	{
		case 1: // RPL_WELCOME
			msg += ":Welcome to the Internet Relay Network ";
			msg += owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost();
			break;
		case 2: // RPL_YOURHOST
			msg += ":Your host is myIRCServer, running version IRC1.0";
			break;
		case 3: // RPL_CREATED
			msg += ":This server was created " + extra;
			break;
		case 4: // RPL_MYINFO
			msg += "myIRCServer IRC1.0 " + UMODES + " " + CMODES;
			break;
		case 221: // RPL_UMODEIS
			msg += owner.getModes();
			break;
		case 251: // RPL_LUSERCLIENT
			msg += ":There are " + extra + " users and 0 invisible on 1 servers";
			break;
		case 252: // RPL_LUSEROP
			msg += "0 :operator(s) online";
			break;
		case 254: // RPL_LUSERCHANNELS
			msg += extra + " :channels formed";
			break;
		case 255: // RPL_LUSERME
			msg += ":I have " + extra + " clients and 1 servers";
			break;
		case 301: // RPL_AWAY
			msg += extra + " :" + owner.getAwayMsg();
			break;
		case 305: // RPL_UNAWAY
			msg += ":You are no longer marked as being away";
			break;
		case 306: // RPL_NOWAWAY
			msg += ":You have been marked as being away";
			break;
		case 315: // RPL_ENDOFNAMES
			msg += extra + " :End of /WHO list";
			break;
		case 321: // RPL_LISTSTART 
			msg += "Channel :Users  Name";
			break;
		case 322: // RPL_LIST 
			msg += extra;
			break;
		case 323: // RPL_LISTEND 
			msg += extra + " :End of /LIST";
			break;
		case 324: // RPL_CHANNELMODEIS 
			msg += extra;
			break;
		case 329: // RPL_CREATIONTIME 
			msg += extra;
			break;
		case 331: // RPL_NOTOPIC
			msg += extra + " :No topic is set";
			break;
		case 332: // RPL_TOPIC
			msg += extra;
			break;
		case 333: // RPL_TOPICWHOTIME
			msg += extra;
			break;
		case 341: // RPL_INVITING
			msg += extra;
			break;
		case 346: // RPL_INVITELIST 
			msg += extra;
			break;
		case 347: // RPL_ENDOFINVITELIST
			msg += extra  + " :End of channel invite list.";
			break;
		case 348: // RPL_EXCEPTLIST 
			msg += extra;
			break;
		case 349: // RPL_ENDOFEXCEPTLIST
			msg += ":End of channel exception list.";
			break;
		case 352: // RPL_WHOREPLY
			msg += extra ;
			break;
		case 353: // RPL_NAMREPLY 
			msg += extra;
			break;
		case 366: // RPL_ENDOFNAMES 
			msg += extra + " :End of /NAMES list";
			break;
		case 367: // RPL_BANLIST
			msg += extra + " :End of channel ban list.";
			break;
		case 368: // RPL_ENDOFBANLIST 
			msg += extra  + " :End of /NAMES list.";
			break;
		case 372: // RPL_MOTD
			msg += ":" + extra;
			break;
		case 375: // RPL_MOTDSTART
			msg += ":- myIRCServer Message of the day - ";
			break;
		case 376: // RPL_ENDOFMOTD
			msg += ":End of /MOTD command.";
			break;
		case 401: // ERR_NOSUCHNICK
			msg += extra + " :No such nick/channel";
			break;
		case 402: // ERR_NOSUCHSERVER
			msg += extra + " :No such server";
			break;
		case 403: // ERR_NOSUCHCHANNEL
			msg += extra + " :No such channel";
			break;
		case 404: // ERR_CANNOTSENDTOCHAN
			msg += extra + " :Cannot send to channel";
			break;
		case 411: // ERR_NORECIPIENT
			msg += ":No recipient given (" + extra + ")";
			break;
		case 412: // ERR_NOTEXTTOSEND
			msg += ":No text to send";
			break;
		case 421: // ERR_UNKNOWNCOMMAND 
			msg += extra + " :Unknown command";
			break;
		case 422: // ERR_NOMOTD
			msg += ":MOTD File is missing";
			break;
		case 431: // ERR_NONICKNAMEGIVEN
			msg += ":No nickname given";
			break;
		case 433: // ERR_NICKNAMEINUSE 
			msg += extra + " :Nickname is already in use"; 
			break;
		case 441: // ERR_USERNOTINCHANNEL  
			msg += extra + " :They aren't on that channel";
			break;
		case 442: // ERR_NOTONCHANNEL 
			msg += extra + " :You're not on that channel";
			break;
		case 443: // ERR_USERONCHANNEL 
			msg += extra + " :is already on channel";
			break;
		case 451: // ERR_NOTREGISTERED
			msg += extra + " :You have not registered";
			break;
		case 461: // ERR_NEEDMOREPARAMS
			msg += extra + " :Not enough parameters"; 
			break;
		case 462: // ERR_ALREADYREGISTERED
			msg += ":You may not reregister"; 
			break;
		case 464: // ERR_PASSWDMISMATCH
			msg += ":Password incorrect";
			break;
		case 471: // ERR_CHANNELISFULL
			msg += extra + " :Cannot join channel (+l)";
			break;
		case 472: // ERR_UNKNOWNMODE
			msg += extra + " :is unknown mode char to me";
			break;
		case 473: // ERR_INVITEONLYCHAN
			msg += extra + " :Cannot join channel (+i)";
			break;
		case 474: // ERR_BANNEDFROMCHAN
			msg += extra + " :Cannot join channel (+b)";
			break;
		case 475: // ERR_BADCHANNELKEY
			msg += extra + " :Cannot join channel (+k)";
			break;
		case 482: // ERR_BADCHANNELKEY
			msg += extra + " :You're not channel operator";
			break;
		case 501: // // ERR_UMODEUNKNOWNFLAG
			msg += ":Unknown MODE flag";
			break;
		case 502: // ERR_USERSDONTMATCH
			msg += ":Cant change mode for other users";
			break;
		case 696: // ERR_INVALIDMODEPARAM
			msg += extra ;
			break;
	}
	msg += "\r\n";
	this->_server.send_msg(msg, owner);
}

void	CommandHandler::_welcome_msg(User& target)
{
	target.set_registered();
	numeric_reply(1, target); // RPL_WELCOME
	numeric_reply(2, target); // RPL_YOURHOST
	numeric_reply(3, target, this->_server.getDateTimeCreated()); // RPL_CREATED
	numeric_reply(4, target); // RPL_MYINFO
	_handleMOTD(target);
	_handleLUSERS(target);
}