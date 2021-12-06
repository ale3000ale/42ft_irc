
#include "CommandHandler.hpp"
#include "Server.hpp"

CommandHandler::CommandHandler(Server	&server): _server(server) {}


void CommandHandler::_parse_cmd(std::string cmd_line)
{
	if (cmd_line.empty())
		return ;
	int pos = cmd_line.find(" ");
	this->_command = cmd_line.substr(0, pos);
	cmd_line.erase(0, (pos != -1) ? pos + 1 : pos);
	this->_params.clear();

	while (!cmd_line.empty())
	{
		if (cmd_line[0] == ':')
		{
			cmd_line.erase(0, 1);
			if (cmd_line.empty())	// TODO: check if this is correct
			{
				this->_params.push_back("");
				break ;
			}
			this->_params.push_back(cmd_line);
			cmd_line.erase(0, -1);
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
		return ; // TODO: send numeric reply
	else if (owner.is_passed() && !owner.is_registered() && this->_command != "NICK" && this->_command != "USER")
		return _numeric_reply(451, owner, this->_command); // ERR_NOTREGISTERED
	// sarebbe figo avere una mappa chiave = comando(es PASS) valore = puntatore funzione corrispondente
	if (this->_command == "PASS")
		_handlePASS(owner);
	else if (this->_command == "NICK")
		_handleNICK(owner);
	else if (this->_command == "USER")
		_handleUSER(owner);
	else if (this->_command == "PING")
		_handlePING(owner);
	else if (this->_command == "JOIN")
		_handleJOIN(owner);
	else if (this->_command == "PART")
		_handlePART(owner);
	else if (this->_command == "PRIVMSG")
		_handlePRIVMSG(owner);
	else if (this->_command == "AWAY")
		_handleAWAY(owner);
	else if (this->_command == "QUIT")
		_handleQUIT(owner);
	else if (this->_command == "WHO")
		_handleWHO(owner);
	else if (this->_command == "KICK")
		_handleKICK(owner);
	else if (this->_command == "MODE")
		_handleMODE(owner);
	else if (this->_command == "TOPIC")
		_handleTOPIC(owner);
	else if (this->_command == "NAMES")
		_handleNAMES(owner);
	else
		_numeric_reply(421, owner, this->_command); // ERR_UNKNOWNCOMMAND
}

void CommandHandler::_handlePASS(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return _numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS
	if (owner.is_registered())
		return _numeric_reply(462, owner); //ERR_ALREADYREGISTERED
	if (this->_server.checkPass(this->_params.front()))
		owner.set_passed();
	else
		_numeric_reply(464, owner); // ERR_PASSWDMISMATCH
}

void CommandHandler::_handleNICK(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return _numeric_reply(431, owner); // ERR_NONICKNAMEGIVEN
	std::string& nick = this->_params.front();
	std::vector<User> const & users = this->_server.getUserList();
	for (u_int i = 0; i < users.size(); i++)
	{
		if (users[i].getNick() == nick)
			return _numeric_reply(433, owner, nick); ; // ERR_NICKNAMEINUS
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
	/*for (std::list<std::string>::iterator i =_params.begin(); i != _params.cend() ; i++)
		std::cout<<*i<<" --- ";*/
	if (this->_params.size() != 4)
		return _numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS
	if (owner.is_registered())
		return _numeric_reply(462, owner); //ERR_ALREADYREGISTERED
	std::string username = this->_params.front();
	if (username.empty()) // i think this case wont ever occur
		return _numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS (461)
	std::string realname = this->_params.back();
	owner.setUsername(username);
	owner.setRealname(realname);
	if (!owner.getNick().empty())
		_welcome_msg(owner);
}

void CommandHandler::_handlePING(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return _numeric_reply(461, owner, this->_command); // ERR_NEEDMOREPARAMS
	//std::cout<<"PONG TEST: "<<this->_params.front()<<std::endl;
	std::string msg = ":myIRCServer PONG myIRCServer :" + this->_params.front() + "\r\n";
	this->_server.send_msg(msg, owner);
}

void CommandHandler::_handlePRIVMSG(User& owner) 
{
	if (!this->_params.size() || this->_params.front() == "")
		return _numeric_reply(411, owner, this->_command); // ERR_NORECIPIENT
	if (this->_params.size() == 1)
		return _numeric_reply(412, owner); // ERR_NOTEXTTOSEND

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
			_numeric_reply(rv, owner, curr_target);
		targets.erase(0, (pos != -1) ? pos + 1 : pos);
	}
}

void CommandHandler::_handleAWAY(User& owner)
{
	if (!this->_params.size())
	{
		owner.setAway(false);
		_numeric_reply(305, owner); // RPL_UNAWAY
	}
	else
	{
		owner.setAway(true, this->_params.front());
		_numeric_reply(306, owner); // RPL_NOWAWAY
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

	while(!names.empty())
	{
		char stat  = 0;
		std::cout << "EXIST?: "<< std::endl;
		if (!_server.exist_channel(names.front()))
		{
			std::cout << "CREATED"<< std::endl;
			Channel ch(names.front(), keys.front(), _server);
			_server.add_channel(ch);
			std::cout << "ADDED"<< std::endl;
			stat = '@';
		}

		Channel &chan = _server.get_channel(names.front());
		std::cout << "JOINING "<< chan.getName() << " key " << chan.getKey() << std::endl;
		//if (keys.empty())
		chan.join_user(owner, keys.front(), stat);
		if (!keys.empty())
			keys.pop_front();
		names.pop_front();
	}
}

void CommandHandler::_handlePART(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return (_numeric_reply(461, owner, this->_command)); // ERR_NEEDMOREPARAMS
	// TODO: codice molot simile in handlePRIVMSG, creare funzione
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
			_numeric_reply(403, owner, curr_target); // ERR_NOSUCHCHANNEL
		else
		{
			Channel& tmp_chan = this->_server.get_channel(curr_target);
			if (!tmp_chan.isInChannel(owner))
				_numeric_reply(442, owner, curr_target); // ERR_NOTONCHANNEL
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
	this->_server.deleteUser(owner.getNick());
}

void CommandHandler::_handleWHO(User& owner) const
{
	const std::vector<User> &us =  _server.getUserList();
	std::string msg;
	Channel ch;
	if (_params.empty())
	{
		//std::cout << "WHO EMPTY\n"; 
		for(size_t i =0 ; i !=us.size(); i++)
		{
			std::cout << "WHO EMPTY " +  us[i].getNick() + " " << i << std::endl; 
			if (!((us[i].commonChannel(owner.getChannels())) || us[i].hasMode('i')) || us[i] == owner)
			{		
				//std::cout << "WHO EMPTY no match "<< i << std::endl; 																// TODO: server.host server.name  					wtf is H/G <hopcount> <real name>
				msg = (us[i].getChannels().empty() ? "" : us[i].getChannels().back()+ " ") + us[i].getUsername() + " " + us[i].getHost() + " myIRCServer " + us[i].getNick() +
				 " H :0 "  + us[i].getRealname();
				_numeric_reply(352, owner, msg);
			}
			//std::cout << "WHO EMPTY SEND " +  us[i].getNick() + " " << i << std::endl; 
		}
		_numeric_reply(315, owner,"*");
	}
	else if(_server.exist_channel(_params.front()))
	{
		ch = _server.get_channel(_params.front());
		bool isInChan = ch.isInChannel(owner);
		const Channel::user_list_type &users = ch.getUserList();
		for (size_t i =0 ;i != users.size(); i++)
		{
			std::cout << "WHO CHAN " +  ch.getName() + " " << i << std::endl;
			if (!isInChan && users[i].second->hasMode('i'))
				continue ;
			// TODO: ci sara un modo per semplificare sto codice OOOOO
			if (users[i].first)
				msg = ch.getName() + " " + users[i].second->getUsername() + " " +  users[i].second->getHost() + " myIRCServer " + users[i].second->getNick() +
				" H" + users[i].first + " :0 " + users[i].second->getRealname();
			else
				msg = ch.getName() + " " + users[i].second->getUsername() + " " +  users[i].second->getHost() + " myIRCServer " + users[i].second->getNick() +
				" H :0 " + users[i].second->getRealname();
			_numeric_reply(352, owner, msg);
		}
		_numeric_reply(315, owner, ch.getName());
	}
}

void	CommandHandler::_handleKICK(User &owner)
{
	if (_params.size() < 2)
	{
		_numeric_reply(461, owner, "KICK");
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
			_numeric_reply(403, owner, channels.front());
		
	}
	

}

/*
	ONLY HANDLING USER MODES
*/
void	CommandHandler::_handleMODE(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return (_numeric_reply(461, owner, this->_command)); // ERR_NEEDMOREPARAMS
	std::string target = this->_params.front();
	if (target[0] == '#') // CHANNEL MODE
	{
		if (!_server.exist_channel(target))
			return (_numeric_reply(403, owner, target)); // ERR_NOSUCHCHANNEL
		Channel &ch = _server.get_channel(target);
		if (this->_params.size() == 1 )
		{
			std::cout << "NO MODES\n";
			_numeric_reply(324, owner, target + " " + _server.get_channel(target).getModes()); // RPL_CHANNELMODEIS
			_numeric_reply(329, owner, target + " " + _server.get_channel(target).getCreationTime()); // RPL_CHANNELMODEIS
			return ;
		}
		_params.pop_front();
		if (ch.isOperator(owner))
		{
			std::cout << "OPERATORE \n";
			std::string mode = _params.front();
			std::cout << "NEW MODE " + mode + "\n";
			_params.pop_front();
			for (size_t i = 1; i < mode.size(); i++)
			{
				std::cout << "MODE " + std::string(1,mode[i]) + " PARAMS "+ _params.front() + "\n";
				if (ch.addMode(owner, mode[i], mode[0], _params.front()))
					_params.pop_front();
			}
		}
		else
			return (_numeric_reply(482, owner, target));
		std::cout << "OPERATORE\n";
		
		
	}
	else	// USER MODE
	{
		std::vector<User> users = this->_server.getUserList();
		uint i=0;
		for (; i<users.size() && users[i].getNick() != target; i++) ;
		if (i == users.size())
			return (_numeric_reply(401, owner, target)); // ERR_NOSUCHNICK
		if (owner.getNick() != target)
			return (_numeric_reply(502, owner)); // ERR_USERSDONTMATCH
		if (this->_params.size() == 1 )
			return (_numeric_reply(221, owner, target)); // RPL_UMODEIS
		std::string modestring = *(++(this->_params.begin()));
		std::string msg = " ";
		for (i=0; i<modestring.length(); i++)
		{
			char mode = modestring[i];
			if (mode == '+' || mode == '-')
				continue;
			if (UMODES.find(mode) == std::string::npos)
				_numeric_reply(501, owner); // ERR_UMODEUNKNOWNFLAG
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
		_numeric_reply(461, owner, "TOPIC");
	else if (!_server.exist_channel(_params.front()))
		_numeric_reply(403, owner, _params.front());
	else 
	{
		Channel &ch = _server.get_channel(_params.front());
		if (_params.size() == 1)
		{
			ch.getTopic(owner);
			
			return;
		}
		_params.pop_front();
		std::cout<< "SIZE TOPIC: " << _params.size()  << " TOPIC -> " +  _params.front() << std::endl;
		ch.setTopic(owner, _params.front());
	}
}

/* void	CommandHandler::_handleLIST(User& owner)
{
	
} */

void	CommandHandler::_handleNAMES(User& owner)
{
	if (_params.size() < 1 || this->_params.front() == "")
	{
		for (std::map<std::string, Channel>::const_iterator i = _server.getchannelList().cbegin();
			i != _server.getchannelList().cend() ; i++)
		{
			std::string msg = "" + (*i).second.getName() + " :"+  (*i).second.getStrUsers() ;
			_numeric_reply(353,owner, msg);
			_numeric_reply(366,owner, (*i).second.getName());
		}
		const std::vector<User> & users = _server.getUserList();
		std::string msg = ":" + owner.getNick() + "!" +  owner.getUsername() + " ";
		for (size_t i = 0; i < users.size() ; i++)
		{
			if (!users[i].getChannels().size())
			{
				msg +=  users[i].getNick() + " * " ;
			}
		}
		msg += "\r\n";
		_server.send_msg( msg, owner);
	}
}	

void	CommandHandler::_numeric_reply(int val, User& owner, std::string extra) const
{
	std::string msg = ":myIRCServer ";

	switch (val)
	{
		case 1: // RPL_WELCOME
			msg += "001 " + owner.getNick() + " :Welcome to the Internet Relay Network ";
			msg += owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost();
			break;
		case 2: // RPL_YOURHOST
			msg += "002 " + owner.getNick() + " :Your host is myIRCServer, running version IRC1.0";
			break;
		case 3: // RPL_CREATED
			msg += "003 " + owner.getNick() + " :This server was created " + extra;
			break;
		case 4: // RPL_MYINFO
			msg += "004 " + owner.getNick() + " myIRCServer IRC1.0 " + UMODES + " " + CMODES;
			break;
		case 221: // RPL_UMODEIS
			msg += "221 " + owner.getNick() + " " + owner.getModes();
			break;
		case 301: // RPL_AWAY
			msg += "301 " + owner.getNick() + " " + extra + " :" + owner.getAwayMsg();
			break;
		case 305: // RPL_UNAWAY
			msg += "305 " + owner.getNick() + " :You are no longer marked as being away";
			break;
		case 306: // RPL_NOWAWAY
			msg += "306 " + owner.getNick() + " :You have been marked as being away";
			break;
		case 315: // RPL_ENDOFNAMES
			msg += "315 " + owner.getNick() + " "  + extra + " :End of /WHO list";
			break;
		case 324: // RPL_CHANNELMODEIS 
			msg += "324 " + owner.getNick() + " "  + extra;
			break;
		case 329: // RPL_CHANNELMODEIS 
			msg += "329 " + owner.getNick() + " "  + extra;
			break;
		case 331: // RPL_NOTOPIC
			msg += "331 " + owner.getNick() + " " + extra + " :No topic is set";
			break;
		case 332: // RPL_TOPIC
			msg += "332 " + owner.getNick() + " " + extra;
			break;
		case 333: // RPL_TOPICWHOTIME
			msg += "333 " + owner.getNick() + " " + extra + "";
			break;
		case 346: // RPL_INVITELIST 
			msg += "346 " + owner.getNick() + " " + extra;
			break;
		case 347: // RPL_ENDOFINVITELIST
			msg += "347 " + owner.getNick() + " " + extra  + " :End of channel invite list.";
			break;
		case 348: // RPL_EXCEPTLIST 
			msg += "348 " + owner.getNick() + " " + extra;
			break;
		case 349: // RPL_ENDOFEXCEPTLIST
			msg += "349 " + owner.getNick() + " " + extra  + " :End of channel exception list.";
			break;
		case 352: // RPL_WHOREPLY
			msg += "352 " + owner.getNick() + " " + extra ; //TODO: \<H|G>[*][@|+] :<hopcount> <real name>"     capire che so
			break;
		case 353: // RPL_NAMREPLY TODO: Remove  _server.get_channel(extra).getStrUsers(); and use only extra
			msg += "353 " + owner.getNick() + extra;
			break;
		case 366: // RPL_ENDOFNAMES 
			msg += "366 " + owner.getNick() + " " + extra + " :End of /NAMES list";
			break;
		case 367: // RPL_BANLIST
			msg += "367 " + owner.getNick() + " " + extra  + " :End of channel ban list.";
			break;
		case 368: // RPL_ENDOFBANLIST 
			msg += "368 " + owner.getNick() + " " + extra  + " :End of /NAMES list.";
			break;
		case 401: // ERR_NOSUCHNICK
			msg += "401 " + owner.getNick() + " " + extra + " :No such nick/channel";
			break;
		case 403: // ERR_NOSUCHCHANNEL
			msg += "403 " + owner.getNick() + " " + extra + " :No such channel";
			break;
		case 404: // ERR_CANNOTSENDTOCHAN
			msg += "404 " + owner.getNick() + " " + extra + " :Cannot send to channel";
			break;
		case 411: // ERR_NORECIPIENT
			msg += "411 " + owner.getNick() + " :No recipient given (" + extra + ")";
			break;
		case 412: // ERR_NOTEXTTOSEND
			msg += "412 " + owner.getNick() + " :No text to send";
			break;
		case 421: // ERR_UNKNOWNCOMMAND 
			msg += "421 " + owner.getNick() + " " + extra + " :Unknown command";
			break;
		case 431: // ERR_NONICKNAMEGIVEN
			msg += "431 " + owner.getNick() + " :No nickname given";
			break;
		case 433: // ERR_NICKNAMEINUSE 
			msg += "433 " + owner.getNick() + " " + extra + " :Nickname is already in use"; 
			break;
		case 441: // ERR_USERNOTINCHANNEL  
			msg += "441 " + owner.getNick() + " " + extra + " :They aren't on that channel";
			break;
		case 442: // ERR_NOTONCHANNEL 
			msg += "442 " + owner.getNick() + " " + extra + " :You're not on that channel";
			break;
		case 451: // ERR_NOTREGISTERED
			msg += "451 " + extra + " :You have not registered";
			break;
		case 461: // ERR_NEEDMOREPARAMS
			msg += "461 " + owner.getNick() + " " + extra + " :Not enough parameters"; 
			break;
		case 462: // ERR_ALREADYREGISTERED
			msg += "462 " + owner.getNick() + " :You may not reregister"; 
			break;
		case 464: // ERR_PASSWDMISMATCH
			msg += "464 " + owner.getHost() + " :Password incorrect";
			break;
		case 475: // ERR_BADCHANNELKEY
			msg += "475 " + owner.getNick() + " " + extra + " :Cannot join channel (+k)";
			break;
		case 482: // ERR_BADCHANNELKEY
			msg += "482 " + owner.getNick() + " " + extra + " :You're not channel operator";
		case 501: // // ERR_UMODEUNKNOWNFLAG
			msg += "501 " + owner.getNick() + " :Unknown MODE flag";
			break;
		case 502: // ERR_USERSDONTMATCH
			msg += "502 " + owner.getNick() + " :Cant change mode for other users";
			break;
		default:
			break;
	}
	msg += "\r\n";
	//std::cout<<"MSG:" + msg;
	this->_server.send_msg(msg, owner);
}

void	CommandHandler::_welcome_msg(User& target) const
{
	target.set_registered();
	_numeric_reply(1, target); // RPL_WELCOME
	_numeric_reply(2, target); // RPL_YOURHOST
	_numeric_reply(3, target, this->_server.getDateTimeCreated()); // RPL_CREATED
	_numeric_reply(4, target); // RPL_MYINFO
	// TODO: LUSERS & MOTD
}