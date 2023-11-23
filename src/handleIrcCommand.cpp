#include "../include/HandleIrcCommand.hpp"
#include "../include/ModeSet.hpp"
#include <algorithm>

static Channel *findChannel(std::string const &name, std::vector<Channel> &channels) {
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].getChannelName() == name)
			return &channels[i];
	}
	return NULL;
}

static size_t split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
	size_t pos = txt.find( ch );
	size_t initialPos = 0;
	strs.clear();

	while( pos != std::string::npos ) {
		strs.push_back( txt.substr( initialPos, pos - initialPos ) );
		initialPos = pos + 1;

		pos = txt.find( ch, initialPos );
	}

	strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

	return strs.size();
}

void handleDCCOffer(const std::string &message, std::vector<User> &users) {
    std::vector<std::string> dccParams;
    split(message, dccParams, ' ');

	int clientSocket;
	std::string targetNick;
	for (size_t i = 0; i < users.size(); i++)
	{
		if (users[i].getNickName() == dccParams[1]) {
			clientSocket = users[i].getSocket();
			targetNick = users[i].getNickName();
			break;
		}
	}

    if (dccParams.size() >= 6) {
        std::string command = dccParams[3];

        if (command == "SEND") {
    		std::string dccRequest = ":" + targetNick + " " + message;
			std::cout << "dccRequest: " << dccRequest;
			send(clientSocket, dccRequest.c_str(), dccRequest.length(), 0);

        } else {
            std::cerr << "Comando DCC non supportato: " << command << std::endl;
        }
    } else {
        std::cerr << "Messaggio DCC non valido: " << message << std::endl;
    }
}

void	handlePassCommand(User &user, std::string const &message, std::string &server_password) {
	std::string pass = message.substr(6);
	pass.erase(pass.length() - 1);

	if (pass != server_password) {
		const char *authErrorMessage = "464 * :Password incorrect\r\n";
		send(user.getSocket(), authErrorMessage, strlen(authErrorMessage), 0);
		close(user.getSocket());
		user.setSocket(0);
	}
}

void	handleNickCommand(User &user, std::string const &message, std::vector<User> &users) {
	std::string newNick = message.substr(5);
	newNick.erase(newNick.length() - 1);
	bool nicknameTaken = false;

	for (size_t i = 0; i < users.size(); i++) {
		if (users[i].getNickName() == newNick) {
			nicknameTaken = true;
			break;
		}
	}

	if (nicknameTaken) {
		std::string nickTakenMessage = "433 * " + newNick + " :Nickname is already in use\r\n";
		send(user.getSocket(), nickTakenMessage.c_str(), strlen(nickTakenMessage.c_str()), 0);
	} else {
		user.setNickName(newNick);
		std::string nickSetMessage = ":IRCSERVER 001 " + newNick + " :Welcome to the IRC server\r\n";
		send(user.getSocket(), nickSetMessage.c_str(), strlen(nickSetMessage.c_str()), 0);
	}
}

void	handleUserCommand(User &user, std::string const &message) {
	std::vector<std::string> userParams;
	split(message, userParams, ' ');

	if (userParams.size() >= 5) {
		std::string username = userParams[1];
		std::string realname = userParams[4];

		user.setUserName(username);
		user.setRealName(realname);

		std::string userSetMessage = ":IRCSERVER 002 " + user.getNickName() + " :Your host is irc.example.com, running version ExampleIRCServer 1.0\r\n";
		send(user.getSocket(), userSetMessage.c_str(), strlen(userSetMessage.c_str()), 0);
		std::string rplCreatedMessage = ":IRCSERVER 003 " + user.getNickName() + " :This server was created in 2023 \r\n";
		send(user.getSocket(), rplCreatedMessage.c_str(), strlen(rplCreatedMessage.c_str()), 0);
		std::string myInfoMessage = ":IRCSERVER 004 " + user.getNickName() + " " + user.getNickName() + " irc.example.com ExampleIRCServer 1.0 " + "+i " + "+xyz" + " [123]\r\n";
        send(user.getSocket(), myInfoMessage.c_str(), myInfoMessage.length(), 0);
		std::string isupportMessage = ":IRCSERVER 005 " + user.getNickName() + " CHANTYPES=#& :are supported by this server\r\n";
        send(user.getSocket(), isupportMessage.c_str(), isupportMessage.length(), 0);
		std::string RPL_MOTD =  ":IRCSERVER 372 " + user.getNickName() + " :- Welcome to the IRCSERVER -\r\n";
        send(user.getSocket(), RPL_MOTD.c_str(), RPL_MOTD.length(), 0);
    	std::string RPL_ENDOFMOTD = ":IRCSERVER 376 " + user.getNickName() + " :End of MOTD command\r\n";
        send(user.getSocket(), RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), 0);
	}
}

void	handlePrivMsgCommand(User &user, std::string const &message, std::vector<User> &users, std::vector<Channel> &channels) {
	std::vector<std::string> splitMessage;
	split(message, splitMessage, ' ');
	std::string nickname = user.getNickName();
	std::string target = splitMessage[1];
	std::string messageText = message.substr(message.find(":", 1) + 1);
	std::string response;
	if (target.at(0) == '#') {
		for (size_t i = 0; i < channels.size(); i++)
		{
			if (channels[i].getChannelName() == target && channels[i].isUserInChannel(user)) {
				std::vector<User> userList = channels[i].getUserList();
				for (size_t j = 0; j < userList.size(); j++) {
					if (nickname != userList[j].getNickName()) {
						response = ":" + nickname + " PRIVMSG " + target + " :" + messageText + "\r\n";
						send(userList[j].getSocket(), response.c_str(), response.length(), 0);
					}
				}
				return;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < users.size(); i++) {
			std::string nick = users[i].getNickName();
			if (nick == target) {
				response = ":" + nickname + " PRIVMSG " + target + " :" + messageText + "\r\n";
				send(users[i].getSocket(), response.c_str(), response.length(), 0);
				return;
			}
		}
	}
}

std::pair<std::string, std::string> parseChannel(std::string const &message)
{
	std::vector<std::string> splitMessage;
	std::string channelName;
	std::string password = "";
	std::cout << message <<std::endl;
	split(message, splitMessage, ' ');
	if(splitMessage.size() != 2)
		channelName = splitMessage[splitMessage.size() - 1];
	else
	{
		channelName = splitMessage[splitMessage.size() -2];
		password = splitMessage[splitMessage.size() - 1];
	}
	return std::pair<std::string, std::string>(channelName, password);
}

std::map<std::string, std::string> parseJoinMsg(std::string const &message)
{
	std::map<std::string, std::string> map;
	std::vector<std::string> splitMessage;
	std::string channelName;
	std::string password;
	if (message.find(',') == std::string::npos)
	{
		split(message, splitMessage, ' ');
		for (size_t i = 1; i < splitMessage.size(); i++)
		{
			if (splitMessage.size() > i + 1)
			{
				if (splitMessage[i].find('#') == 0 && splitMessage[i + 1].find('#') == 0){
					map.insert(parseChannel(splitMessage[i] + ""));
				}else{
					map.insert(parseChannel(splitMessage[i] + " " + splitMessage[i + 1]));
					i++;
				}
			}
			else if (splitMessage[i].find('#') == 0 )
				map.insert(parseChannel(splitMessage[i] + ""));
		}
	}
	else
	{
		std::vector<std::string> subSplit;
		split(message, subSplit, ',');
		//std::cout <<"str : " << subSplit[0].find(' ')+1 << " : " << subSplit[0].size() << std::endl;
		subSplit[0] = subSplit[0].substr(subSplit[0].find(' ')+1, subSplit[0].size());
		//std::cout <<"str : " << subSplit[0] << std::endl;
		for (size_t i = 0; i < subSplit.size(); i++)
		{
			map.insert(parseChannel(subSplit[i]));
		}
		
	}
	//map.insert(std::pair<std::string, std::string>(splitMessage[1], splitMessage[2]));
	return map;
}

void	handleJoinCommand(User &user, std::string const &message, std::vector<Channel> &channels) {
	std::string msgCopy(message);
	msgCopy.erase(msgCopy.length() - 1);
	std::map<std::string, std::string> map = parseJoinMsg(msgCopy);
	std::map<std::string, std::string>::iterator  it = map.begin();
	//std::cout << "message" << it->first << std::endl;
	//(void)user;
	//(void)channels;
	//std::vector<std::string> splitMessage;
	//split(message, splitMessage, ' ');
	//std::string channelName;
	//if(splitMessage.size() == 2)
	//	channelName = splitMessage[splitMessage.size() - 1].erase(splitMessage[splitMessage.size() - 1].length() - 1);
	//else
	//	channelName = splitMessage[splitMessage.size() -2];

	for ( ; it != map.end(); it++)
	{
		std::cout << it->first << " : " << it->second << std::endl;
	
		Channel *existingChannel = findChannel(it->first, channels);

		if (existingChannel) {
			if (!existingChannel->checkUserLimit()) {
				std::string inviteOnlyErrorMessage = ":* 471 " + user.getNickName() + " " + it->first + ":Cannot join channel (+l)\r\n";
				send(user.getSocket(), inviteOnlyErrorMessage.c_str(), strlen(inviteOnlyErrorMessage.c_str()), 0);
				return;
			}
			if (existingChannel->getInviteOnly()) {
				if (!existingChannel->isInvitedUser(user)) {
					std::string inviteOnlyErrorMessage = ":* 473 " + user.getNickName() + " " + it->first + " :Channel is invite-only\r\n";
					send(user.getSocket(), inviteOnlyErrorMessage.c_str(), strlen(inviteOnlyErrorMessage.c_str()), 0);
					return;
				}
			}
			std::string providedPassword;
			if (it->second != "")
				providedPassword = it->second;
			else
				providedPassword = "";
			// std::cout << "provided password:" << providedPassword << std::endl;
			if (existingChannel->checkChannelPassword(providedPassword)) {
				existingChannel->addUser(user);
			} else {
				std::string passwordIncorrectMessage = ":* 475 " + user.getNickName() + " " + it->first + " :Password incorrect\r\n";
				send(user.getSocket(), passwordIncorrectMessage.c_str(), strlen(passwordIncorrectMessage.c_str()), 0);
				return;
			}
			//
			existingChannel->addUser(user);
			std::string joinMessage = ":" + user.getNickName() + " JOIN " + it->first + "\r\n";
			send(user.getSocket(), joinMessage.c_str(), joinMessage.length(), 0);

			existingChannel->handleJoinMessage(user);

			if (existingChannel->getTopic() == "") {
				std::string topicMessage = ":" + user.getNickName() + " 331 " + user.getNickName() + " " + existingChannel->getChannelName() + " :No topic is set" + "\r\n";
				send(user.getSocket(), topicMessage.c_str(), topicMessage.length(), 0);
			} else {
				std::string topicMessage = ":" + user.getNickName() + " 332 " + user.getNickName() + " " + existingChannel->getChannelName() + " :" + existingChannel->getTopic() + "\r\n";
				send(user.getSocket(), topicMessage.c_str(), topicMessage.length(), 0);
			}

			existingChannel->updateUserList(user);
		} else {
			Channel newChannel(it->first);
			newChannel.addUser(user);
			newChannel.addOperators(user);
			channels.push_back(newChannel);

			std::string joinMessage = ":" + user.getNickName() + " JOIN " + it->first + "\r\n";
			send(user.getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
			std::string userListMessage = ": 353 " + user.getNickName() + " = " + it->first + " :" + user.getNickName() + "\r\n";
			send(user.getSocket(), userListMessage.c_str(), userListMessage.length(), 0);
			std::string modeOperator = "MODE " + it->first + " +o " + user.getNickName() + "\r\n";
			send(user.getSocket(), modeOperator.c_str(), modeOperator.length(), 0);
		}
	}
}

void handlePartCommand(User &user, std::string const &message, std::vector<Channel> &channels) {
    std::vector<std::string> splitMessage;
    split(message, splitMessage, ' ');
    std::string channelName = splitMessage[1];
    Channel *channel = findChannel(channelName, channels);

    if (channel) {
        std::string partMessage = ":" + user.getNickName() + "!" + user.getNickName() + " PART :" + channel->getChannelName() + "\r\n";
        channel->broadcastMessage(partMessage);
        channel->removeUser(user);
		channel->updateUserList(user);
    } else {
        std::string partErrorMessage = "403 " + user.getNickName() + " " + channelName + " :No such channel\r\n";
        send(user.getSocket(), partErrorMessage.c_str(), strlen(partErrorMessage.c_str()), 0);
        return;
    }
}

void	handleInviteCommand(User &user, std::string const &message, std::vector<User> &users, std::vector<Channel> &channels) {
	std::vector<std::string> splitMessage;
	split(message, splitMessage, ' ');
	std::string invitedNick = splitMessage[1];

	std::string channelName = splitMessage[splitMessage.size() - 1];
	channelName.erase(channelName.length() - 1);
	Channel *channel = findChannel(channelName, channels);

	if (channel) {
		if (channel->inviteUser(user, invitedNick, channelName, users)) {
			std::string inviteResponse = ": 341 " + user.getNickName() + " " + invitedNick + " " + channelName + " :Invitation sent\r\n";
			send(user.getSocket(), inviteResponse.c_str(), inviteResponse.length(), 0);
		} else {
			std::string inviteErrorMessage = ": 443 " + user.getNickName() + " " + channelName + " " + invitedNick + " :Cannot invite user\r\n";
			send(user.getSocket(), inviteErrorMessage.c_str(), inviteErrorMessage.length(), 0);
		}
	}
}

void handleModeCommand(User &user, std::string const &message, std::vector<Channel> &channels) {
    std::vector<std::string> splitMessage;
    split(message, splitMessage, ' ');

    if (splitMessage.size() < 2) {
        std::string modeErrorMessage = "461 " + user.getNickName() + " MODE :Not enough parameters\r\n";
        send(user.getSocket(), modeErrorMessage.c_str(), strlen(modeErrorMessage.c_str()), 0);
        return;
    }

    if (splitMessage.size() > 2) {
        std::string channelName = splitMessage[1];
        Channel *targetChannel = findChannel(channelName, channels);

        for (size_t i = 2; i < splitMessage.size(); ++i) {
            char val = splitMessage[i][1];
			char sign = splitMessage[i][0];
            switch (val) {
                case 'i':
                    modSetInviteOnly(targetChannel, sign, user);
                    break;
                case 't':
                    modSetTopicRestrictions(targetChannel, sign, user);
                    break;
                case 'k':
					if (sign == '+') {
						if (i + 1 < splitMessage.size() && !splitMessage[i + 1].empty())
							modeSetChannelKey(targetChannel, splitMessage[i + 1], user);
					}
					else if (sign == '-')
						modeRemoveChannelKey(targetChannel, user);
                    break;
                case 'o':
					if (i + 1 < splitMessage.size() && !splitMessage[i + 1].empty()) {
						if (sign == '+')
							modeSetChannelOperator(targetChannel, splitMessage[i + 1], user);
						else if (sign == '-')
							modeRemoveChannelOperator(targetChannel, splitMessage[i + 1], user);
					}
                    break;
                case 'l':
					if (sign == '+') {
						if (i + 1 < splitMessage.size() && !splitMessage[i + 1].empty())
							modSetLimit(targetChannel, splitMessage[i + 1], user);
					}
					else if (sign == '-')
						modeRemoveLimit(targetChannel, user);
                    break;
                default:
                    break;
            }
        }
    }
}

void	handleTopicCommand(User &user, std::string const &message, std::vector<Channel> &channels) {
	std::vector<std::string> splitMessage;
	split(message, splitMessage, ' ');
	std::string channelName = splitMessage[1];
	std::string newTopic = message.substr(message.find(":", 1) + 1);
	Channel *channel = findChannel(channelName, channels);
	if (channel) {
		if (channel->isOperator(user) || channel->getTopicRestriction() == false) {
			channel->setTopic(newTopic);
			std::string topicMessage = ":" + user.getNickName() + " 332 " + user.getNickName() + " " + channel->getChannelName() + " :" + channel->getTopic() + "\r\n";
			channel->broadcastMessage(topicMessage);
		} else {
			const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
			send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
		}
		return;
	}
	else {
		const char *noChannelMessage = ":403 * :No such channel\r\n";
		send(user.getSocket(), noChannelMessage, strlen(noChannelMessage), 0);
	}
}

void	handlePingCommand(User &user, std::string const &message) {
	std::vector<std::string> splitMessage;
	split(message, splitMessage, ' ');
	std::string token = splitMessage[1];
	token.erase(token.length() - 1);
	user.setToken(token);
 	std::string pongMessage = "PONG :" + user.getToken() + "\r\n";
    send(user.getSocket(), pongMessage.c_str(), pongMessage.length(), 0);
}

void handleKickCommand(User &user, std::vector<User> &users, std::string const &message, std::vector<Channel> &channels)
{
	std::vector<std::string> splitMsg;
	Channel *chn = NULL;
	User *target = NULL;
	std::cout << "msg " << message << std::endl;
	split(message, splitMsg, ' ');
	std::string channelName = splitMsg[1];
	for (size_t i = 0; i < users.size(); i++)
	{
		if (users[i].getNickName() == splitMsg[2])
		{
			target = &(users[i]);
			break;
		}
	}
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channelName == channels[i].getChannelName())
		{
			chn = &(channels[i]);
			break;
		}
	}
	if (chn == NULL)
	{
		std::string noSuchChannel = ": * 403 " + user.getNickName() + " " + channelName + " :No such channel\r\n";
		send(user.getSocket(), noSuchChannel.c_str(), noSuchChannel.size(), 0);
		return;
	}
	if (!chn->isUserInChannel(user))
	{
		std::string userNotInChannel = ": * 441 " + target->getNickName() + " " + chn->getChannelName() + " :User not in channel\r\n";
		send(user.getSocket(), userNotInChannel.c_str(), userNotInChannel.size(), 0);
		return;
	}
	if (!chn->isOperator(user))
	{
		std::string operatorPrivilegesNeeded = ": * 482 " + user.getNickName() + " " + chn->getChannelName() + " :You're not channel operator\r\n";
		send(user.getSocket(), operatorPrivilegesNeeded.c_str(), operatorPrivilegesNeeded.size(), 0);
		return;
	}
	std::string request = "KICK " + chn->getChannelName() + " " + splitMsg[2];
	std::string str = RPL_KICK(user.getNickName(), user.getUserName(), chn->getChannelName(), target->getNickName());
	send(target->getSocket(), str.c_str(), str.size(), MSG_DONTWAIT);
	chn->removeUser(*target);
	chn->broadcastMessage(str);
	std::cout << "User " << splitMsg[2] << " kicked from channel " << chn->getChannelName() << std::endl;
}

void handleQuitCommand(User &user, std::vector<User> &users, std::vector<Channel> &channels) {
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].isUserInChannel(user)) {
			channels[i].removeUser(user);
			if (!channels[i].isEmpty()) {
				std::string partMessage = ":" + user.getNickName() + "!" + user.getNickName() + " PART :" + channels[i].getChannelName() + "\r\n";
				channels[i].broadcastMessage(partMessage);
				channels[i].updateUserList(user);
			}
		}
	}
	std::vector<User>::iterator it = std::find(users.begin(), users.end(), user);
    if (it != users.end()) {
        users.erase(it);
    }
}
