#pragma once
#include "../include/User.hpp"
#include <sys/socket.h>
#include <iostream>
#include <sstream>
#include <cstdio>


class IrcBot
{
public:
    void replyInfo(User user);
    void infoMessage(User user);
    IrcBot();
    ~IrcBot();
};

# define RPL_INFO_MESSAGE(nickName, realName, userName, socket) "NOTICE " + nickName + " : User info\nSocket " + socket + "\nnickName " + nickName+ "\nrealName " + realName + "\nuserName " + userName + "\r\n"
# define RPL_INFO(nick) "NOTICE " + nick + " :!info for displaying user info" + "\r\n"