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

void modSetInviteOnly(Channel *targetChannel ,std::string &modeChange) {
    if (modeChange == "+i") {
        // Imposta il canale in modalità "invite only"
        targetChannel->setInviteOnly(true);
        std::string modeChangeMessage = "MODE " + targetChannel->getChannelName() + " +i\r\n";
        targetChannel->broadcastMessage(modeChangeMessage);
    } else if (modeChange == "-i") {
        // Rimuovi la modalità "invite only" dal canale
        targetChannel->setInviteOnly(false);
        std::string modeChangeMessage = "MODE " + targetChannel->getChannelName() + " -i\r\n";
        targetChannel->broadcastMessage(modeChangeMessage);
    }
}

void modSetLimit(Channel *targetChannel ,std::string &modeChange, std::vector<std::string> splitMessage, User user) {
    if (modeChange == "+l") {
        std::string limit = splitMessage[3];
        limit.erase(limit.length() - 1);
        targetChannel->setUserLimit(atoi(limit.c_str()));
        // Invia un messaggio di conferma all'utente
        std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " +l " + limit + "\r\n";
        send(user.getSocket(), modeConfirmation.c_str(), strlen(modeConfirmation.c_str()), 0);
    }
    else if (modeChange == "-l") {
        // Rimuovi il limite degli utenti
        targetChannel->removeUserLimit();
        // Invia un messaggio di conferma all'utente
        std::string modeConfirmation = "MODE " + targetChannel->getChannelName() + " -l\r\n";
        send(user.getSocket(), modeConfirmation.c_str(), strlen(modeConfirmation.c_str()), 0);
    }
}