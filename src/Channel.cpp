#include "../include/Channel.hpp"
#include <sys/socket.h>
#include <algorithm>
#include <iostream>

Channel::Channel(std::string name) : channelName(name), inviteOnly(false), userLimit(-1)
{}

std::string Channel::getChannelName() const
{
    return channelName;
}

std::string Channel::getTopic() const {
    return this->topic;
}

void    Channel::setTopic(std::string &topic) {
    this->topic = topic;
}

void Channel::addUser(User user)
{
    bool find = false;
    for (size_t i = 0; i < userList.size(); i++)
    {
        if (userList[i].getNickName() == user.getNickName())
            find = true;
    }
    if (find == false)
        userList.push_back(user);
}

void Channel::addOperators(User user)
{
    bool find = false;
    for (size_t i = 0; i < operators.size(); i++)
    {
        if (operators[i].getNickName() == user.getNickName())
            find = true;
    }
    if (find == false)
        operators.push_back(user);
}

std::vector<User>   Channel::getUserList() const {
    return this->userList;
}

bool    Channel::isOperator(User user) const {
    for (size_t i = 0; i < operators.size(); i++)
    {
        if (operators[i].getNickName() == user.getNickName())
            return true;
    }
    return false;
}

bool    Channel::isUserInChannel(User user) const {
    for (size_t i = 0; i < userList.size(); i++)
    {
        if (userList[i].getNickName() == user.getNickName())
            return true;
    }
    return false;
}

bool Channel::inviteUser(User user, std::string const &invitedNick, std::string const &channelName,std::vector<User> &users) {
    User *invitedUser = NULL;

    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].getNickName() == invitedNick) {
            invitedUser = &users[i];
            break;;
        }
    }

    if (invitedUser) {
        if (!this->isOperator(user))
            return false;
        if (this->isUserInChannel(*invitedUser))
            return false;

        this->addInvitesUser(*invitedUser);
        std::string inviteMessage = ": INVITE " + invitedUser->getNickName() + " :You have been invited by " + user.getNickName() + " to join the channel " + channelName + "\r\n";
        send(invitedUser->getSocket(), inviteMessage.c_str(), inviteMessage.length(), 0);

        return true;
    }

    return false;
}

bool    Channel::kickUser(User user, std::string const &targetNick, std::vector<User> &users) {
    User *targetUser = NULL;

    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].getNickName() == targetNick) {
            targetUser = &users[i];
            break;;
        }
    }
    if (!this->isOperator(user))
        return false;

    if (targetUser) {
        std::cout << "target user find: " << targetUser->getNickName() << " socket: " << targetUser->getSocket() << std::endl;
        // Invia un messaggio di "KICK" all'utente espulso
        std::string kickMessage = ":" + user.getNickName() + " KICK " + channelName + " " + targetNick + " :You were kicked from the channel\r\n";
        send(targetUser->getSocket(), kickMessage.c_str(), kickMessage.length(), 0);

        // Rimuovi l'utente espulso dal canale
        for (size_t i = 0; i < userList.size(); i++)
        {
            if (targetNick == userList[i].getNickName()) {
                userList.erase(userList.begin() + i);
                std::cout << "remove: " << userList[i].getNickName() << std::endl;
                break;
            }
        }

        return true; // Espulsione riuscita
    }

    return false; // Utente target non trovato nel canale
}

void Channel::broadcastMessage(const std::string &message) {
    for (size_t i = 0; i < userList.size(); i++) {
        send(userList[i].getSocket(), message.c_str(), message.length(), 0);
    }
}

void Channel::removeUser(User user) {
    for (std::vector<User>::iterator it = userList.begin(); it != userList.end(); ++it) {
        if (it->getNickName() == user.getNickName()) {
            userList.erase(it);
            break;
        }
    }
}

bool Channel::getInviteOnly() const {
    return this->inviteOnly;
}

void Channel::setInviteOnly(bool value) {
    this->inviteOnly = value;
}

void Channel::addInvitesUser(User user) {
    bool find = false;
    for (size_t i = 0; i < invitedUsers.size(); i++)
    {
        if (invitedUsers[i].getNickName() == user.getNickName())
            find = true;
    }
    if (find == false)
        invitedUsers.push_back(user);
}

bool Channel::isInvitedUser(User user) {
    for (size_t i = 0; i < invitedUsers.size(); i++)
    {
        if (user.getNickName() == invitedUsers[i].getNickName())
            return true;
    }
    return false;
}

bool Channel::checkUserLimit() {
    if (this->userLimit != -1) {
        if ((int)this->userList.size() > this->userLimit)
            return false;
    }
    return true;
}

void Channel::setUserLimit(int limit) {
    this->userLimit = limit;
}

void Channel::removeUserLimit() {
    this->userLimit = -1;
}

Channel::~Channel() { }
