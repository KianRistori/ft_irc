#pragma once

#include "../include/Channel.hpp"
#include "../include/User.hpp"

void modSetInviteOnly(Channel *targetChannel, char sign, User user);
void modSetLimit(Channel *targetChannel, std::vector<std::string> splitMessage, char sign, User user);
void modSetTopicRestrictions(Channel *targetChannel, char sign, User user);
void modeSetChannelKey(Channel *targetChannel, std::vector<std::string> splitMessage, char sign, User user);
void modeSetChannelOperator(Channel *targetChannel, std::vector<std::string> splitMessage, char sign, User user);