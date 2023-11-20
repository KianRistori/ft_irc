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

void modSetLimit(Channel *targetChannel, std::vector<std::string> splitMessage, char sign, User user) {
    if (targetChannel->isOperator(user)) {
        if (sign == '+') {
            splitMessage[splitMessage.size() - 1].erase(splitMessage[splitMessage.size() - 1].length() - 1);
            int maxSize = std::atoi(splitMessage[splitMessage.size() - 1].c_str());
            targetChannel->setUserLimit(maxSize);
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " +l " + splitMessage[splitMessage.size() - 1] + "\r\n";
            targetChannel->broadcastMessage(modeConfirmation);
        }
        else if (sign == '-') {
            targetChannel->removeUserLimit();
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " -l\r\n";
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

void modeSetChannelKey(Channel *targetChannel, std::vector<std::string> splitMessage, char sign, User user) {
    if (targetChannel->isOperator(user)) {
        if (sign == '+') {
            splitMessage[splitMessage.size() - 1].erase(splitMessage[splitMessage.size() - 1].length() - 1);
            targetChannel->setPassword(splitMessage[splitMessage.size() - 1]);
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " +k " + splitMessage[splitMessage.size() - 1] + "\r\n";
            targetChannel->broadcastMessage(modeConfirmation);
        }
        else if (sign == '-') {
            targetChannel->setPassword("");
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " -k \r\n";
            targetChannel->broadcastMessage(modeConfirmation);
        }
    }
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}

void modeSetChannelOperator(Channel *targetChannel, std::vector<std::string> splitMessage, char sign, User user) {
    if (targetChannel->isOperator(user)) {
        if (sign == '+') {
            splitMessage[splitMessage.size() - 1].erase(splitMessage[splitMessage.size() - 1].length() - 1);
            std::string targetName = splitMessage[splitMessage.size() - 1];
            User *userTarget = targetChannel->findUserInChannel(targetName);
            targetChannel->addOperators(*userTarget);
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " +o " + userTarget->getNickName() + "\r\n";
            targetChannel->broadcastMessage(modeConfirmation);
            delete userTarget;
        }
        else if (sign == '-') {
            splitMessage[splitMessage.size() - 1].erase(splitMessage[splitMessage.size() - 1].length() - 1);
            std::string targetName = splitMessage[splitMessage.size() - 1];
            User *userTarget = targetChannel->findUserInChannel(targetName);
            targetChannel->removeOperator(*userTarget);
            std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " -o " + userTarget->getNickName() + "\r\n";
            targetChannel->broadcastMessage(modeConfirmation);
        }
    }
    else {
        const char *noPermissionMessage = ":482 * :You're not channel operator\r\n";
        send(user.getSocket(), noPermissionMessage, strlen(noPermissionMessage), 0);
    }
}