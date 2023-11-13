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

void modSetInviteOnly(Channel *targetChannel, char sign) {
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

void modSetLimit(Channel *targetChannel, std::vector<std::string> splitMessage, char sign) {
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

void modSetTopicRestrictions(Channel *targetChannel, char sign) {
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

void modeChannelKey(Channel *targetChannel, std::vector<std::string> splitMessage, char sign) {
    if (sign == '+') {
        splitMessage[splitMessage.size() - 1].erase(splitMessage[splitMessage.size() - 1].length() - 1);
        // std::cout << "password: " << splitMessage[splitMessage.size() - 1] << std::endl;
        targetChannel->setPassword(splitMessage[splitMessage.size() - 1]);
        std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " +k \r\n";
        targetChannel->broadcastMessage(modeConfirmation);
    }
    else if (sign == '-') {
        targetChannel->setPassword("");
        std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " -k \r\n";
        targetChannel->broadcastMessage(modeConfirmation);
    }
}