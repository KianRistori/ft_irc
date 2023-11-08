#include "../include/HandleIrcCommand.hpp"

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

	// Decompose statement
	while( pos != std::string::npos ) {
		strs.push_back( txt.substr( initialPos, pos - initialPos ) );
		initialPos = pos + 1;

		pos = txt.find( ch, initialPos );
	}

	// Add the last one
	strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

	return strs.size();
}

void	handlePassCommand(User &user, std::string const &message, std::string &server_password) {
	std::string pass = message.substr(6);
	pass.erase(pass.length() - 1);

	// Verifica se la password è corretta
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

	// Verifica se il nickname è già in uso da un altro utente
	for (size_t i = 0; i < users.size(); i++) {
		if (users[i].getNickName() == newNick) {
			nicknameTaken = true;
			break;
		}
	}

	if (nicknameTaken) {
		// Invia un messaggio di errore
		std::string nickTakenMessage = "433 * " + newNick + " :Nickname is already in use\r\n";
		send(user.getSocket(), nickTakenMessage.c_str(), strlen(nickTakenMessage.c_str()), 0);
	} else {
		// Imposta il nuovo nickname per l'utente
		user.setNickName(newNick);

		// Invia una conferma al client
		std::string nickSetMessage = "001 " + newNick + " :Welcome to the IRC server\r\n";
		send(user.getSocket(), nickSetMessage.c_str(), strlen(nickSetMessage.c_str()), 0);
	}
}

void	handleUserCommand(User &user, std::string const &message) {
	std::vector<std::string> userParams;
	split(message, userParams, ' ');

	if (userParams.size() >= 5) {
		// userParams[1] contiene il nome utente
		// userParams[4] contiene il nome reale dell'utente

		std::string username = userParams[1];
		std::string realname = userParams[4];

		// Puoi salvare il nome utente e il nome reale dell'utente nei dati dell'oggetto User
		user.setUserName(username);
		user.setRealName(realname);

		// Invia una conferma al client
		std::string userSetMessage = "002 " + user.getNickName() + " :Your host is irc.example.com, running version ExampleIRCServer 1.0\r\n";
		send(user.getSocket(), userSetMessage.c_str(), strlen(userSetMessage.c_str()), 0);
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
			if (channels[i].getChannelName() == target) {
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
	std::string channelName = splitMessage[splitMessage.size() - 1].erase(splitMessage[splitMessage.size() - 1].length() - 1);

	// Verifica se il canale esiste già
	Channel *existingChannel = findChannel(channelName, channels);

	if (existingChannel) {
		// Il canale esiste già, quindi aggiungi l'utente al canale
		existingChannel->addUser(user);
		std::cout << user.getNickName() << " join " << existingChannel->getChannelName() << std::endl;

		// Invia il messaggio "JOIN" al client Konversation
		std::string joinMessage = ":" + user.getNickName() + " JOIN " + channelName + "\r\n";
		std::cout << "joinMessage: " << joinMessage << std::endl;
        existingChannel->broadcastMessage(joinMessage);

		// Invia il topic attuale del canale al nuovo utente
		std::string topicMessage = ":" + user.getNickName() + " 332 " + user.getNickName() + " " + existingChannel->getChannelName() + " :" + existingChannel->getTopic() + "\r\n";
		send(user.getSocket(), topicMessage.c_str(), topicMessage.length(), 0);

		 // Ottieni la lista degli utenti e inviala al client
		std::vector<User> userList = existingChannel->getUserList();
		std::string userListMessage = ": 353 " + user.getNickName() + " = " + channelName + " :";
		for (size_t i = 0; i < userList.size(); i++) {
			userListMessage += userList[i].getNickName();
			if (i < userList.size() - 1) {
				userListMessage += " ";
			}
		}
		userListMessage += "\r\n";
		existingChannel->broadcastMessage(userListMessage);
	} else {
		// Il canale non esiste, quindi crealo e aggiungi l'utente
		Channel newChannel(channelName);
		newChannel.addUser(user);
		newChannel.addOperators(user);
		channels.push_back(newChannel);

		// Invia il messaggio "JOIN" al client Konversation
		std::string joinMessage = ":" + user.getNickName() + " JOIN " + channelName + "\r\n";
		send(user.getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
		std::string userListMessage = ": 353 " + user.getNickName() + " = " + channelName + " :" + user.getNickName() + "\r\n";
		send(user.getSocket(), userListMessage.c_str(), userListMessage.length(), 0);
	}
}

void	handlePartCommand(User &user, std::string const &message, std::vector<Channel> &channels) {
	std::vector<std::string> splitMessage;
    split(message, splitMessage, ' ');
	std::string channelName = splitMessage[1];
	Channel *channel = findChannel(channelName, channels);

	if (channel) {
		channel->removeUser(user);
		std::string partMessage = ":" + user.getNickName() + " PART " + channel->getChannelName() + "\r\n";
		channel->broadcastMessage(partMessage);
		std::string partConfirmation = "PART " + channelName + "\r\n";
    	send(user.getSocket(), partConfirmation.c_str(), strlen(partConfirmation.c_str()), 0);
	} else {
		std::string partErrorMessage = "403 " + user.getNickName() + " " + channelName + " :No such channel\r\n";
        send(user.getSocket(), partErrorMessage.c_str(), strlen(partErrorMessage.c_str()), 0);
        return;
	}
}

void	handleInviteCommand(User &user, std::string const &message, std::vector<User> &users, std::vector<Channel> &channels) {
	// Estrai il nome utente invitato dalla richiesta di invito
	std::vector<std::string> splitMessage;
	split(message, splitMessage, ' ');
	std::string invitedNick = splitMessage[1];

	// Cerca il canale a cui si applica l'invito
	std::string channelName = splitMessage[splitMessage.size() - 1];
	channelName.erase(channelName.length() - 1);
	Channel *channel = findChannel(channelName, channels);

	if (channel) {
		// Prova ad invitare l'utente al canale
		if (channel->inviteUser(user, invitedNick, channelName, users)) {
			// Invito riuscito, invia una risposta al client che ha inviato l'invito
			std::string inviteResponse = ": 341 " + user.getNickName() + " " + invitedNick + " " + channelName + " :Invitation sent\r\n";
			send(user.getSocket(), inviteResponse.c_str(), inviteResponse.length(), 0);
		} else {
			// Invito fallito, invia un messaggio di errore
			std::string inviteErrorMessage = ": 443 " + user.getNickName() + " " + channelName + " " + invitedNick + " :Cannot invite user\r\n";
			send(user.getSocket(), inviteErrorMessage.c_str(), inviteErrorMessage.length(), 0);
		}
	}
}

void	handleTopicCommand(User &user, std::string const &message, std::vector<Channel> &channels) {
	std::vector<std::string> splitMessage;
	split(message, splitMessage, ' ');
	std::string channelName = splitMessage[1];
	std::string newTopic = message.substr(message.find(":", 1) + 1);
	// Cerco il canale
	Channel *channel = findChannel(channelName, channels);
	if (channel) {
		if (channel->isOperator(user)) {
			// Cambio il topic del canale
			channel->setTopic(newTopic);

			// Invio una notifica agli utenti del canale
			std::string topicMessage = ":" + user.getNickName() + " 332 " + user.getNickName() + " " + channel->getChannelName() + " :" + channel->getTopic() + "\r\n";
			channel->broadcastMessage(topicMessage);
		} else {
			// L'utente non ha i permessi per cambiare il topic
			const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
			send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
		}
		return;
	}
	else {
		// Se il canale non esiste
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
		if (channelName == channels[i].getChannelName() /*&& channels[i].isUserInChannel(user) && channels[i].isOperator(user)*/)
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
	std::cout << "send : " << request << std::endl;
	send(target->getSocket(), request.c_str(), request.size(), 0);
	chn->removeUser(*target);
	chn->broadcastMessage("User " + splitMsg[2] + " has been kicked by " + user.getNickName() + "\r\n");
	std::cout << "User " << splitMsg[2] << " kicked from channel " << chn->getChannelName() << std::endl;
}