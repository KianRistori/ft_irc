#pragma once

#include "../include/Channel.hpp"
#include "../include/User.hpp"

void modSetInviteOnly(Channel *targetChannel, char sign);
void modSetLimit(Channel *targetChannel, std::vector<std::string> splitMessage, char sign);
void modSetTopicRestrictions(Channel *targetChannel, char sign);
void modeChannelKey(Channel *targetChannel, std::vector<std::string> splitMessage, char sign);