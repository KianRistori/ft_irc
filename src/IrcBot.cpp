#include "../include/IrcBot.hpp"

IrcBot::IrcBot()
{}

void IrcBot::replyInfo(User user)
{
    std::stringstream ss;
    ss << user.getSocket();
    std::string sockStr = ss.str();
    std::string msg = RPL_INFO_MESSAGE(user.getNickName(), user.getRealName(), user.getUserName(), sockStr);
    if (!send(user.getSocket(), msg.c_str(), msg.size(), MSG_DONTWAIT))
    {
        perror("send replyInfo");
    }
}

void IrcBot::infoMessage(User user)
{
    std::string msg = RPL_INFO(user.getNickName());
    if (!send(user.getSocket(), msg.c_str(), msg.size(), MSG_DONTWAIT))
    {
        perror("send replyInfo");
    }
}

IrcBot::~IrcBot() {}