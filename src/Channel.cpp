#include "../include/Channel.hpp"
#include <sys/socket.h>
#include <algorithm>
#include <iostream>

Channel::Channel(std::string name) : channelName(name), inviteOnly(false), topicRestriction(true), userLimit(-1)
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

void Channel::broadcastMessage(const std::string &message) {
    for (size_t i = 0; i < userList.size(); i++) {
        send(userList[i].getSocket(), message.c_str(), message.length(), MSG_DONTWAIT);
    }
}

void Channel::removeUser(User user) {
    std::vector<User>::iterator it = std::find(userList.begin(), userList.end(), user);
    if (it != userList.end()) {
        // std::cout << "kicked user : " << (*it).getNickName() << std::endl;
        userList.erase(it);
        // for (size_t i = 0; i < userList.size(); i++)
        // {
        //     std::cout << i << " : " << userList[i].getNickName() << std::endl;
        // }
        
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

void Channel::handleJoinMessage(User user) {
    for (size_t i = 0; i < userList.size(); i++)
    {
        if (user.getNickName() != userList[i].getNickName()) {
            std::string joinMessage = ":" + user.getNickName() + "!" + user.getNickName() + " JOIN " + this->getChannelName() + "\r\n";
		    send(userList[i].getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
        }
    }
}

void Channel::updateUserList(User user) {
    std::string userListMessage = ": 353 " + user.getNickName() + " = " + this->getChannelName() + " :";
		for (size_t i = 0; i < userList.size(); i++) {
			userListMessage += userList[i].getNickName();
			if (i < userList.size() - 1) {
				userListMessage += " ";
			}
		}
    userListMessage += "\r\n";
    this->broadcastMessage(userListMessage);
}

void Channel::setPassword(std::string password) {
    this->password = password;
}

bool Channel::checkChannelPassword(std::string password) {
    std::cout << "provided pass: " << password << " | pass: " << this->password << std::endl;
    if (this->password == "")
        return true;
    else if (this->password == password)
        return true;
    return false;
}

void Channel::setTopicRestriction(bool topicRestriction) {
    this->topicRestriction = topicRestriction;
}

bool Channel::getTopicRestriction() {
    return this->topicRestriction;
}

Channel::~Channel() { }
