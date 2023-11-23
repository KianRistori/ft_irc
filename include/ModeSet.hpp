#pragma once

#include "../include/Channel.hpp"
#include "../include/User.hpp"

void modSetInviteOnly(Channel *targetChannel, char sign, User user);
void modSetLimit(Channel *targetChannel, std::string &limit, User user);
void modSetTopicRestrictions(Channel *targetChannel, char sign, User user);
void modeSetChannelKey(Channel *targetChannel, std::string &password, User user);
void modeSetChannelOperator(Channel *targetChannel, std::string &targetName, User user);
void modeRemoveChannelKey(Channel *targetChannel, User user);
void modeRemoveChannelOperator(Channel *targetChannel, std::string &targetName, User user);
void modeRemoveLimit(Channel *targetChannel, User user);