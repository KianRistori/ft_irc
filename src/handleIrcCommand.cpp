#include "../include/HandleIrcCommand.hpp"
#include "../include/ModeSet.hpp"

static Channel *findChannel(std::string &name, std::vector<Channel> &channels) {
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
		std::string nickSetMessage = "001 " + newNick + " :Welcome to the IRC server\r\n";
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

		std::string userSetMessage = "002 " + user.getNickName() + " :Your host is irc.example.com, running version ExampleIRCServer 1.0\r\n";
		send(user.getSocket(), userSetMessage.c_str(), strlen(userSetMessage.c_str()), 0);
	}
}

void handleFileTransfer(std::vector<std::string> &strs) {
    std::string clientAddress = strs[5];
    int dccPort = atoi(strs[6].c_str());
    int dccClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in dccClientAddr;
    dccClientAddr.sin_family = AF_INET;
    dccClientAddr.sin_addr.s_addr = inet_addr(clientAddress.c_str());
    dccClientAddr.sin_port = htons(dccPort);

    if (connect(dccClientSocket, (struct sockaddr*)&dccClientAddr, sizeof(dccClientAddr)) == -1) {
        std::cerr << "Errore nella connessione al client DCC." << std::endl;
        close(dccClientSocket);
        return;
    }

    std::string targetNickName = strs[1];
    std::string filename = "/home/kian/example/file.txt";
    int bufferSize = atoi(strs[strs.size() - 1].erase(strs[strs.size() - 1].length() - 1).c_str());

    // Fase di negoziazione
    std::string negotiationMessage = "DCC RESUME " + filename + " " + strs[6] + " 0";  // Imposta position come 0, puoi adattarlo secondo necessità
    send(dccClientSocket, negotiationMessage.c_str(), negotiationMessage.length(), 0);

    // Ricevi la risposta del client DCC
    char responseBuffer[1024];  // Dimensione arbitraria per la risposta
    ssize_t bytesRead = recv(dccClientSocket, responseBuffer, sizeof(responseBuffer), 0);
    if (bytesRead == -1) {
        std::cerr << "Errore nella ricezione della risposta del client DCC." << std::endl;
        close(dccClientSocket);
        return;
    }

    responseBuffer[bytesRead] = '\0';  // Assicura che il buffer sia terminato correttamente

    // Analizza la risposta
    std::string response(responseBuffer);

    if (response.find("DCC ACCEPT") == 0) {
        // Invia la conferma di accettazione
        send(dccClientSocket, response.c_str(), response.length(), 0);
    } else {
        std::cout << "Trasferimento DCC annullato dal client." << std::endl;
        close(dccClientSocket);
        return;
    }

    // Ricevi il file dal client DCC
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file: " << filename << std::endl;
        close(dccClientSocket);
        return;
    }

    char buffer[bufferSize];
    while (file.read(buffer, bufferSize) || file.gcount() > 0) {
        ssize_t sentBytes = send(dccClientSocket, buffer, file.gcount(), 0);
        if (sentBytes == -1) {
            std::cerr << "Errore nell'invio del file." << std::endl;
            break;
        }
    }

    file.close();

    std::cout << "Trasferimento DCC completato. File ricevuto: " << filename << std::endl;

    // Chiudi la connessione con il client DCC
    close(dccClientSocket);
}

void	handlePrivMsgCommand(User &user, std::string const &message, std::vector<User> &users, std::vector<Channel> &channels) {
	std::vector<std::string> strs;
	split(message, strs, ' ');

	for (size_t i = 0; i < strs.size(); i++)
	{
		if (strs[i].compare(":DCC") && strs[i + 1].compare("SEND")) {
			handleFileTransfer(strs);
			return;
		}
	}
	
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

void	handleJoinCommand(User &user, std::string const &message, std::vector<Channel> &channels) {
	std::vector<std::string> splitMessage;
	split(message, splitMessage, ' ');
	std::string channelName;
	if(splitMessage.size() == 2)
		channelName = splitMessage[splitMessage.size() - 1].erase(splitMessage[splitMessage.size() - 1].length() - 1);
	else
		channelName = splitMessage[splitMessage.size() -2];

	Channel *existingChannel = findChannel(channelName, channels);

	if (existingChannel) {
		if (!existingChannel->checkUserLimit()) {
			std::string inviteOnlyErrorMessage = ":* 471 " + user.getNickName() + " " + channelName + ":Cannot join channel (+l)\r\n";
			send(user.getSocket(), inviteOnlyErrorMessage.c_str(), strlen(inviteOnlyErrorMessage.c_str()), 0);
			return;
		}
		if (existingChannel->getInviteOnly()) {
			if (!existingChannel->isInvitedUser(user)) {
				std::string inviteOnlyErrorMessage = ":* 473 " + user.getNickName() + " " + channelName + " :Channel is invite-only\r\n";
                send(user.getSocket(), inviteOnlyErrorMessage.c_str(), strlen(inviteOnlyErrorMessage.c_str()), 0);
                return;
            }
		}
		std::string providedPassword;
		if (splitMessage.size() == 3)
			providedPassword = splitMessage[2].erase(splitMessage[2].size() - 1);
		else
			providedPassword = "";
		std::cout << "provided password : " << providedPassword << std::endl;
		if (existingChannel->checkChannelPassword(providedPassword)) {
			existingChannel->addUser(user);
		} else {
			std::string passwordIncorrectMessage = ":* 475 " + user.getNickName() + " " + channelName + " :Password incorrect\r\n";
			send(user.getSocket(), passwordIncorrectMessage.c_str(), strlen(passwordIncorrectMessage.c_str()), 0);
			return;
		}
		//
		existingChannel->addUser(user);
		std::string joinMessage = ":" + user.getNickName() + " JOIN " + channelName + "\r\n";
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
		Channel newChannel(channelName);
		newChannel.addUser(user);
		newChannel.addOperators(user);
		channels.push_back(newChannel);

		std::string joinMessage = ":" + user.getNickName() + " JOIN " + channelName + "\r\n";
		send(user.getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
		std::string userListMessage = ": 353 " + user.getNickName() + " = " + channelName + " :" + user.getNickName() + "\r\n";
		send(user.getSocket(), userListMessage.c_str(), userListMessage.length(), 0);
		std::string modeOperator = "MODE " + channelName + " +o " + user.getNickName() + "\r\n";
		send(user.getSocket(), modeOperator.c_str(), modeOperator.length(), 0);
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
	if (splitMessage.size() > 2)
	{
		std::string channelName = splitMessage[1];
		Channel *targetChannel = findChannel(channelName, channels);
		char val = splitMessage[2][1];
		switch (val)
		{
		case 'i':
				modSetInviteOnly(targetChannel, splitMessage[2][0], user);
			break;
		case 't':
				modSetTopicRestrictions(targetChannel, splitMessage[2][0], user);
			break;
		case 'k':
				modeSetChannelKey(targetChannel, splitMessage, splitMessage[2][0], user);
			break;
		case 'o':
				modeSetChannelOperator(targetChannel, splitMessage, splitMessage[2][0], user);
			break;
		case 'l':
				modSetLimit(targetChannel, splitMessage, splitMessage[2][0], user);
			break;
		default:
			break;
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
	std::string request = "KICK " + chn->getChannelName() + " " + splitMsg[2] + ":reason\r\n";
	std::string str = RPL_KICK(user.getNickName(), user.getUserName(), chn->getChannelName(), target->getNickName(), "reason_blbbl");
	send(target->getSocket(), str.c_str(), str.size(), MSG_DONTWAIT);
	chn->removeUser(*target);
	chn->broadcastMessage(str);
	std::cout << "User " << splitMsg[2] << " kicked from channel " << chn->getChannelName() << std::endl;
}