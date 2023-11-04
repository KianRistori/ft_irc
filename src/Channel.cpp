#include "../include/Channel.hpp"

Channel::Channel(std::string name) : channelName(name)
{}

std::string Channel::getChannelName() const
{
    return channelName;
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

std::vector<User>   Channel::getUserList() const {
    return this->userList;
}

Channel::~Channel() { }
