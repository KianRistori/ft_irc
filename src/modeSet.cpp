#include "../include/ModeSet.hpp"
#include "../include/User.hpp"
#include "../include/Channel.hpp"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <vector>
#include <algorithm>

static bool checkPassword(const std::string &str) {
    if (str.empty())
        return true;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        if (!std::isspace(*it) && *it != '\r' && *it != '\n') {
            return false;
        }
    }
    return true;
}

void modSetInviteOnly(Channel *targetChannel, char sign, User user) {
    if (targetChannel->isOperator(user)) {
        if (sign == '+') {
            targetChannel->setInviteOnly(true);
            std::string modeChangeMessage = "MODE " + targetChannel->getChannelName() + " +i\r\n";
            targetChannel->broadcastMessage(modeChangeMessage);
        } else if (sign == '-') {
            targetChannel->setInviteOnly(false);
            std::string modeChangeMessage = "MODE " + targetChannel->getChannelName() + " -i\r\n";
            targetChannel->broadcastMessage(modeChangeMessage);
        }
    }
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}

void modSetLimit(Channel *targetChannel, std::string &limit, User user) {
    if (targetChannel->isOperator(user)) {
        int maxSize = std::atoi(limit.c_str());
        if (maxSize != 0) {
            targetChannel->setUserLimit(maxSize);
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " +l " + limit + "\r\n";
            targetChannel->broadcastMessage(modeConfirmation);
        }
    }
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}

void modSetTopicRestrictions(Channel *targetChannel, char sign, User user) {
    if (targetChannel->isOperator(user)) { 
        if (sign == '+') {
            targetChannel->setTopicRestriction(true);
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " +t \r\n";
            targetChannel->broadcastMessage(modeConfirmation);
        }
        else if (sign == '-') {
            targetChannel->setTopicRestriction(false);
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " -t \r\n";
            targetChannel->broadcastMessage(modeConfirmation);
        }
    } 
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}

void modeSetChannelKey(Channel *targetChannel, std::string &password,User user) {
    if (targetChannel->isOperator(user)) {
        if (!checkPassword(password)) {
            targetChannel->setPassword(password);
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " +k " + password + "\r\n";
            targetChannel->broadcastMessage(modeConfirmation);
        }
        else {
            const char *passwordErrorMessage = ":400 * :Password required for this operation\r\n";
            send(user.getSocket(), passwordErrorMessage, strlen(passwordErrorMessage), 0);
        }
    }
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}

void modeRemoveChannelKey(Channel *targetChannel, User user) {
    if (targetChannel->isOperator(user)) {
        targetChannel->setPassword("");
        std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " -k \r\n";
        targetChannel->broadcastMessage(modeConfirmation);
    }
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}

void modeSetChannelOperator(Channel *targetChannel, std::string &targetName, User user) {
    if (targetChannel->isOperator(user)) {
        User *userTarget = targetChannel->findUserInChannel(targetName);
        targetChannel->addOperators(*userTarget);
        std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " +o " + userTarget->getNickName() + "\r\n";
        targetChannel->broadcastMessage(modeConfirmation);
    }
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}

void modeRemoveChannelOperator(Channel *targetChannel, std::string &targetName, User user) {
    if (targetChannel->isOperator(user)) {
        User *userTarget = targetChannel->findUserInChannel(targetName);
        targetChannel->removeOperator(*userTarget);
        std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " -o " + userTarget->getNickName() + "\r\n";
        targetChannel->broadcastMessage(modeConfirmation);
    }
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}

void modeRemoveLimit(Channel *targetChannel, User user) {
    if (targetChannel->isOperator(user)) {
        targetChannel->removeUserLimit();
        std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " -l\r\n";
        targetChannel->broadcastMessage(modeConfirmation);
    }
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}